#include "model.h"
#include "access_gate.h"
#include "persistence.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace inventory_next;
namespace fs = std::filesystem;
namespace {
std::string quoted(const std::string& s) {
    std::string out="\"";
    for(unsigned char c:s) {
        if(c=='"'||c=='\\') {out+='\\';out+=static_cast<char>(c);}
        else if(c<32) { const char* h="0123456789abcdef";out+="\\u00";out+=h[c>>4];out+=h[c&15]; }
        else out+=static_cast<char>(c);
    }
    return out+'"';
}
void require(bool ok,const std::string& reason) {if(!ok) throw std::runtime_error(reason);}
template<typename T> T number(const std::string& s) {
    T result{}; auto r=std::from_chars(s.data(),s.data()+s.size(),result);
    require(r.ec==std::errc{} && r.ptr==s.data()+s.size(),"invalid_number");return result;
}
bool boolean(const std::string& s) {auto n=number<unsigned>(s);require(n<2,"invalid_boolean");return n!=0;}
bool safe_name(const std::string& s) {
    if(s.empty()||s.size()>48) return false;
    for(char c:s) if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-')) return false;
    std::string upper=s;
    for(char& c:upper) if(c>='a'&&c<='z') c=static_cast<char>(c-'a'+'A');
    if(upper=="CON"||upper=="PRN"||upper=="AUX"||upper=="NUL") return false;
    if(upper.size()==4 && (upper.substr(0,3)=="COM"||upper.substr(0,3)=="LPT") && upper[3]>='1'&&upper[3]<='9') return false;
    return true;
}
struct Lab {
    Model model;
    AccessGate access{model};
    std::uint64_t captured_access=0;
    bool batching=false;
    bool relocating=false;
    std::vector<Command> batch;
    std::map<std::string,Node> definitions;
    std::vector<Node> seeds;
    std::ofstream events;
    fs::path output;
    fs::path initial;
    std::string current_command;
    bool begun=false;
    std::size_t steps=0;
    std::uint64_t captured_session=0;
    void event(std::size_t line,const std::string& op,const std::string& actual,bool pass, Mass projected=-1) {
        events<<"{\"line\":"<<line<<",\"op\":"<<quoted(op)<<",\"actual\":"<<quoted(actual)
              <<",\"command\":"<<quoted(current_command)
              <<",\"pass\":"<<(pass?"true":"false")<<",\"revision\":"<<model.revision()
              <<",\"session\":"<<model.session()<<",\"high_water\":"<<model.high_water_id()
              <<",\"carried_grams\":"<<model.carried_mass()<<",\"projected_grams\":"<<projected<<"}\n";
        events.flush();require(static_cast<bool>(events),"log_write");
    }
    void line(std::vector<std::string> t,std::size_t index) {
        if(t.empty()) return;
        current_command.clear();
        for(const auto& token:t) { if(!current_command.empty()) current_command+=' '; current_command+=token; }
        auto count=[&](std::size_t n){require(t.size()==n,"argument_count");};
        if(t[0]=="pouch_definition"||t[0]=="container_stack_limit"||t[0]=="attribute_definition"||t[0]=="compartment_name"||t[0]=="mount_definition") {
            require(!begun&&t.size()>=2,"definition_after_begin");require(definitions.count(t[1])!=0,"unknown_definition");Node& n=definitions.at(t[1]);
            if(t[0]=="pouch_definition"){count(2);n.pouch=true;}
            else if(t[0]=="container_stack_limit"){count(3);n.max_stack_quantity=number<std::uint32_t>(t[2]);}
            else if(t[0]=="attribute_definition"){count(4);n.attributes[t[2]]=t[3];}
            else if(t[0]=="compartment_name"){count(4);n.compartment_names[number<unsigned>(t[2])]=t[3];}
            else {count(9);MountSlot slot;slot.id=number<Id>(t[2]);slot.name=t[3];slot.footprint={number<int>(t[4]),number<int>(t[5])};slot.accepts=number<std::uint64_t>(t[6]);slot.minimum_condition=number<unsigned>(t[7]);slot.enabled=boolean(t[8]);n.mounts.push_back(slot);}
            return;
        }
        if(t[0]=="cells_definition"||t[0]=="root_cells") {
            require(!begun&&t.size()>=3,"cell_definition");auto size=number<std::size_t>(t[2]);
            require(size<=4096&&t.size()==size+3,"cell_definition");std::vector<unsigned> cells;
            for(std::size_t i=3;i<t.size();++i)cells.push_back(number<unsigned>(t[i]));
            if(t[0]=="cells_definition") {require(definitions.count(t[1])!=0,"unknown_definition");definitions.at(t[1]).cells=std::move(cells);}
            else {bool found=false;Id id=number<Id>(t[1]);for(Node& n:seeds)if(n.id==id){n.cells=cells;found=true;}require(found,"policy_root");}
            return;
        }
        if(t[0]=="stack_definition"||t[0]=="wear_definition") {
            require(!begun,"definition_after_begin");count(6);
            require(definitions.count(t[1])!=0,"unknown_definition");Node& n=definitions.at(t[1]);
            if(t[0]=="stack_definition") {
                n.type=number<Id>(t[2]);n.state=number<Id>(t[3]);
                n.quantity=number<std::uint32_t>(t[4]);n.stack_limit=number<std::uint32_t>(t[5]);
            } else {
                n.wear_slots=number<std::uint64_t>(t[2]);n.required_slots=number<unsigned>(t[3]);
                n.blocked_slots=number<std::uint64_t>(t[4]);n.presentation_group=number<unsigned>(t[5]);
            }
            return;
        }
        if(t[0]=="equipment_policy") {
            require(!begun,"policy_after_begin");count(5);Id id=number<Id>(t[1]);bool found=false;
            for(Node& n:seeds) if(n.id==id&&n.kind==Kind::ActorRoot) {
                n.allowed_slots=number<std::uint64_t>(t[2]);n.equipment_accepts=number<std::uint64_t>(t[3]);
                n.equipment_capacity=number<unsigned>(t[4]);found=true;
            }
            require(found,"policy_root");return;
        }
        if(t[0]=="definition") {
            require(!begun,"definition_after_begin");count(16);require(safe_name(t[1]),"definition_name");
            Node n;n.intrinsic=number<Mass>(t[2]);n.category=number<std::uint64_t>(t[3]);
            n.empty={number<int>(t[4]),number<int>(t[5])};n.half={number<int>(t[6]),number<int>(t[7])};
            n.full={number<int>(t[8]),number<int>(t[9])};n.container=boolean(t[10]);n.soft=boolean(t[11]);
            n.grid={number<int>(t[12]),number<int>(t[13])};n.accepts=number<std::uint64_t>(t[14]);
            n.max_contents=number<Mass>(t[15]);require(definitions.emplace(t[1],n).second,"duplicate_definition");return;
        }
        if(t[0]=="root") {
            require(!begun,"root_after_begin");count(7);Node n;n.id=number<Id>(t[1]);
            require(t[2]=="actor"||t[2]=="world","root_kind");n.kind=t[2]=="actor"?Kind::ActorRoot:Kind::WorldRoot;
            n.category=0;n.container=true;n.grid={number<int>(t[3]),number<int>(t[4])};
            n.max_contents=number<Mass>(t[5]);n.accepts=number<std::uint64_t>(t[6]);seeds.push_back(n);return;
        }
        if(t[0]=="seed") {
            require(!begun,"seed_after_begin");count(7);require(definitions.count(t[3])!=0,"unknown_definition");
            Node n=definitions.at(t[3]);n.id=number<Id>(t[1]);n.parent=number<Id>(t[2]);
            n.position={number<int>(t[4]),number<int>(t[5]),boolean(t[6])};seeds.push_back(n);return;
        }
        if(t[0]=="begin") {
            count(1);require(!begun,"duplicate_begin");
            require(initial.empty()||seeds.empty(),"resume_with_seeds");
            auto r=initial.empty()?model.initialize(seeds):load_file(model,initial);
            require(r.ok,"fixture_"+r.error);
            begun=true;captured_session=model.session();event(index,"begin","ok",true);return;
        }
        require(begun,"begin_required");++steps;
        if(t[0]=="batch_begin"||t[0]=="layout_begin") {
            count(1);require(!batching,"nested_batch");batching=true;relocating=t[0]=="layout_begin";batch.clear();return;
        }
        if(t[0]=="batch_end"||t[0]=="layout_end") {
            count(2);require(batching,"batch_required");auto before=model.serialize();auto epoch=model.session();
            require((t[0]=="layout_end")==relocating,"batch_mode");
            std::vector<Move> moves;
            for(const auto& c:batch) moves.push_back({c.item,c.destination,c.position,c.revision,c.session});
            auto r=relocating?model.relocate(moves):model.execute_batch(batch);std::string actual=r.ok?"ok":r.error;
            if(!r)require(model.serialize()==before&&model.session()==epoch,"rollback_invariant");
            event(index,"batch_end",actual,actual==t[1]);batching=false;batch.clear();require(actual==t[1],"unexpected_result");return;
        }
        if(batching) require(t[0]=="spawn"||t[0]=="move"||t[0]=="mass"||t[0]=="erase"||t[0]=="split"||t[0]=="merge"||t[0]=="equip"||t[0]=="mount"||t[0]=="state","batch_command_only");
        if(batching&&relocating)require(t[0]=="move","layout_move_only");
        if(t[0]=="assert") {
            require(t.size()>=3,"assert_syntax");bool pass=false;
            if(t[1]=="mass") {count(3);pass=model.carried_mass()==number<Mass>(t[2]);}
            else if(t[1]=="count") {count(3);pass=model.nodes().size()==number<std::size_t>(t[2]);}
            else if(t[1]=="revision") {count(3);pass=model.revision()==number<std::uint64_t>(t[2]);}
            else if(t[1]=="high_water") {count(3);pass=model.high_water_id()==number<Id>(t[2]);}
            else if(t[1]=="world_count") {count(3);pass=model.world_items().size()==number<std::size_t>(t[2]);}
            else if(t[1]=="accessible_count") {count(4);pass=access.children(number<Id>(t[2])).size()==number<std::size_t>(t[3]);}
            else if(t[1]=="parent") {count(4);auto n=model.nodes().find(number<Id>(t[2]));pass=n!=model.nodes().end()&&n->second.parent==number<Id>(t[3]);}
            else if(t[1]=="quantity") {count(4);auto n=model.nodes().find(number<Id>(t[2]));pass=n!=model.nodes().end()&&n->second.quantity==number<unsigned>(t[3]);}
            else if(t[1]=="equipped") {count(4);auto n=model.nodes().find(number<Id>(t[2]));pass=n!=model.nodes().end()&&n->second.location==Location::Equipment&&n->second.equipped_slots==number<std::uint64_t>(t[3]);}
            else if(t[1]=="mounted") {count(4);auto n=model.nodes().find(number<Id>(t[2]));pass=n!=model.nodes().end()&&n->second.location==Location::Mounted&&n->second.mounted_slot==number<Id>(t[3]);}
            else if(t[1]=="group_count") {count(4);pass=model.equipment_group(number<unsigned>(t[2])).size()==number<std::size_t>(t[3]);}
            else if(t[1]=="cycle") {count(6);pass=model.cycle_equipment(number<unsigned>(t[2]),number<Id>(t[3]),number<int>(t[4]))==number<Id>(t[5]);}
            else if(t[1]=="footprint") {count(5);auto s=model.footprint(number<Id>(t[2]));pass=s.w==number<int>(t[3])&&s.h==number<int>(t[4]);}
            else throw std::runtime_error("unknown_assert");
            event(index,"assert_"+t[1],pass?"ok":"assertion",pass);require(pass,"assertion");return;
        }
        if(t[0]=="save"||t[0]=="load") {
            count(2);require(safe_name(t[1]),"snapshot_name");
            auto path=output/(t[1]+".inv");
            auto r=t[0]=="save"?save_atomic(model,path):load_file(model,path);
            event(index,t[0],r.ok?"ok":r.error,r.ok);require(r.ok,r.error);return;
        }
        if(t[0]=="capture_session") {count(1);captured_session=model.session();return;}
        if(t[0]=="capture_access") {count(1);captured_access=access.revision();return;}
        if(t[0]=="observe") {
            count(4);auto r=access.observe(number<Id>(t[1]),boolean(t[2]),boolean(t[3]));
            event(index,"observe",r.ok?"ok":r.error,r.ok);require(r.ok,r.error);return;
        }
        if(t[0]=="reveal") {
            count(4);auto r=access.observe_item(number<Id>(t[1]),boolean(t[2]),boolean(t[3]));
            event(index,"reveal",r.ok?"ok":r.error,r.ok);require(r.ok,r.error);return;
        }
        if(t[0]=="snapshot") {
            count(2);require(safe_name(t[1]),"snapshot_name");
            std::ofstream file(output/("snapshot-"+t[1]+".json"));require(static_cast<bool>(file),"snapshot_open");
            file<<"{\"revision\":"<<model.revision()<<",\"carried_grams\":"<<model.carried_mass()<<",\"items\":[";
            bool first=true;
            for(const auto& n:model.snapshot()) {
                if(!first)file<<',';
                first=false;
                file<<"{\"id\":"<<n.id<<",\"parent\":"<<n.parent<<",\"type\":"<<n.type<<",\"state\":"<<n.state
                    <<",\"quantity\":"<<n.quantity<<",\"equipped\":"<<(n.location==Location::Equipment?"true":"false")
                    <<",\"overflow\":"<<(n.location==Location::Overflow?"true":"false")<<",\"slots\":"<<n.equipped_slots<<",\"mount\":"<<n.mounted_slot<<",\"condition\":"<<n.condition<<",\"group\":"<<n.presentation_group<<",\"x\":"<<n.position.x
                    <<",\"y\":"<<n.position.y<<",\"width\":"<<n.footprint.w<<",\"height\":"<<n.footprint.h
                    <<",\"gross_grams\":"<<n.gross_mass<<'}';
            }
            file<<"]}\n";file.flush();require(static_cast<bool>(file),"snapshot_write");event(index,"snapshot","ok",true);return;
        }
        bool preview=false,old_session=false,guarded=false,old_access=false;
        if(t[0]=="preview") {preview=true;t.erase(t.begin());require(!t.empty(),"preview_command");}
        if(t[0]=="captured") {old_session=true;t.erase(t.begin());require(!t.empty(),"captured_command");}
        if(t[0]=="guarded"||t[0]=="guarded_captured") {
            guarded=true;old_access=t[0]=="guarded_captured";t.erase(t.begin());require(!t.empty(),"guarded_command");
        }
        Command c;c.session=old_session?captured_session:model.session();
        std::string op=t[0];
        if(op=="spawn"||op=="recover_spawn") {
            count(9);c.operation=Operation::Spawn;require(definitions.count(t[2])!=0,"unknown_definition");
            c.spawned=definitions.at(t[2]);c.spawned.id=number<Id>(t[1]);c.spawned.parent=number<Id>(t[3]);
            c.spawned.position={number<int>(t[4]),number<int>(t[5]),boolean(t[6])};c.revision=number<std::uint64_t>(t[7]);
        } else if(op=="move") {
            count(8);c.operation=Operation::Move;c.item=number<Id>(t[1]);c.destination=number<Id>(t[2]);
            c.position={number<int>(t[3]),number<int>(t[4]),boolean(t[5])};c.revision=number<std::uint64_t>(t[6]);
        } else if(op=="mass") {
            count(5);c.operation=Operation::SetMass;c.item=number<Id>(t[1]);c.mass=number<Mass>(t[2]);c.revision=number<std::uint64_t>(t[3]);
        } else if(op=="erase") {
            count(4);c.operation=Operation::Erase;c.item=number<Id>(t[1]);c.revision=number<std::uint64_t>(t[2]);
        } else if(op=="split") {
            count(10);c.operation=Operation::Split;c.item=number<Id>(t[1]);c.quantity=number<std::uint32_t>(t[2]);c.new_id=number<Id>(t[3]);
            c.destination=number<Id>(t[4]);c.position={number<int>(t[5]),number<int>(t[6]),boolean(t[7])};c.revision=number<std::uint64_t>(t[8]);
        } else if(op=="merge") {
            count(6);c.operation=Operation::Merge;c.item=number<Id>(t[1]);c.destination=number<Id>(t[2]);
            c.quantity=number<std::uint32_t>(t[3]);c.revision=number<std::uint64_t>(t[4]);
        } else if(op=="equip") {
            count(6);c.operation=Operation::Equip;c.item=number<Id>(t[1]);c.destination=number<Id>(t[2]);
            c.slots=number<std::uint64_t>(t[3]);c.revision=number<std::uint64_t>(t[4]);
        } else if(op=="mount") {
            count(6);c.operation=Operation::Mount;c.item=number<Id>(t[1]);c.destination=number<Id>(t[2]);c.mount_slot=number<Id>(t[3]);c.revision=number<std::uint64_t>(t[4]);
        } else if(op=="state") {
            count(7);c.operation=Operation::SetState;c.item=number<Id>(t[1]);c.mass=number<Mass>(t[2]);c.state=number<Id>(t[3]);c.condition=number<unsigned>(t[4]);c.revision=number<std::uint64_t>(t[5]);
            auto item=model.nodes().find(c.item);if(item!=model.nodes().end())c.attributes=item->second.attributes;
        } else throw std::runtime_error("unknown_command");
        if(batching) {
            require(t.back()=="queued"&&batch.size()<64,"batch_queue");batch.push_back(c);event(index,op,"queued",true);return;
        }
        auto before=model.serialize();auto epoch=model.session();Result r{false,{}};Mass projected=-1;
        AccessGate::Ticket ticket{c,old_access?captured_access:access.revision()};
        if(op=="recover_spawn") {require(!guarded&&!preview,"explicit_recovery_only");r=model.recover_native_arrival(c.spawned,c.revision,c.session);}
        else if(preview) {auto p=guarded?access.preview(ticket):model.preview(c);r=p.result;projected=p.carried_mass;}
        else r=guarded?access.execute(ticket):model.execute(c);
        std::string actual=r.ok?"ok":r.error;bool pass=actual==t.back();
        if(preview||!r) require(model.serialize()==before&&model.session()==epoch,"rollback_invariant");
        event(index,(preview?"preview_":"")+op,actual,pass,projected);require(pass,"unexpected_result");
    }
};
}
int main(int argc,char** argv) {
    fs::path output;std::ofstream status;std::size_t lines=0;Lab lab;
    auto start=std::chrono::steady_clock::now();
    try {
        require(SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS)!=0,"priority");
        fs::path input;
        if(argc==1) {
            wchar_t module[32768];
            DWORD length=GetModuleFileNameW(nullptr,module,32768);
            require(length>0&&length<32768,"module_path");
            auto folder=fs::path(module).parent_path();
            input=folder/"scenarios"/"lifecycle.sqa";
            auto runs=folder/"runs";fs::create_directories(runs);
            output=runs/("run-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
        } else {
            require((argc==5||argc==7)&&std::string(argv[1])=="--scenario"&&std::string(argv[3])=="--output","arguments");
            if(argc==7) {require(std::string(argv[5])=="--resume","arguments");lab.initial=fs::absolute(argv[6]);}
            input=fs::absolute(argv[2]);output=fs::path(argv[4]);
        }
        require(output.is_absolute(),"output_absolute_required");
        require(fs::create_directory(output),"output_already_exists");lab.output=output;
        status.open(output/"result.json",std::ios::binary);require(static_cast<bool>(status),"status_open");
        lab.events.open(output/"events.jsonl",std::ios::binary);require(static_cast<bool>(lab.events),"log_open");
        require(fs::file_size(input)<=128*1024,"scenario_size");std::ifstream file(input);
        require(static_cast<bool>(file),"scenario_open");std::string line;
        require(static_cast<bool>(std::getline(file,line)),"scenario_header");
        if(!line.empty()&&line.back()=='\r') line.pop_back();
        require(line=="SQA3_SCENARIO 1","scenario_header");
        while(std::getline(file,line)) {
            ++lines;require(lines<=512&&line.size()<=2048,"scenario_limit");
            require(std::chrono::steady_clock::now()-start<std::chrono::seconds(30),"timeout");
            auto comment=line.find('#');if(comment!=std::string::npos)line.resize(comment);
            std::istringstream s(line);std::vector<std::string> tokens;std::string token;
            while(s>>token)tokens.push_back(token);
            lab.line(std::move(tokens),lines+1);
        }
        require(file.eof(),"scenario_read");require(lab.begun&&lab.steps>0,"empty_scenario");require(!lab.batching,"unclosed_batch");
        auto saved=save_atomic(lab.model,output/"final.inv");require(saved.ok,saved.error);
        Model restored;auto loaded=load_file(restored,output/"final.inv");
        require(loaded.ok&&restored.serialize()==lab.model.serialize(),"final_reload");
        status<<"{\"success\":true,\"steps\":"<<lab.steps<<",\"revision\":"<<lab.model.revision()
              <<",\"carried_grams\":"<<lab.model.carried_mass()<<",\"priority_class\":"<<GetPriorityClass(GetCurrentProcess())
              <<",\"console_attached\":"<<(GetConsoleWindow()?"true":"false")<<",\"game_launched\":false}\n";
        status.flush();require(static_cast<bool>(status),"status_write");return 0;
    } catch(const std::exception& e) {
        if(status) {status<<"{\"success\":false,\"line\":"<<lines+1<<",\"error\":"<<quoted(e.what())<<"}\n";status.flush();}
        return 1;
    }
}
