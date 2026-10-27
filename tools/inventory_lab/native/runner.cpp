#include "model.h"
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
        if(t[0]=="assert") {
            require(t.size()>=3,"assert_syntax");bool pass=false;
            if(t[1]=="mass") {count(3);pass=model.carried_mass()==number<Mass>(t[2]);}
            else if(t[1]=="count") {count(3);pass=model.nodes().size()==number<std::size_t>(t[2]);}
            else if(t[1]=="revision") {count(3);pass=model.revision()==number<std::uint64_t>(t[2]);}
            else if(t[1]=="high_water") {count(3);pass=model.high_water_id()==number<Id>(t[2]);}
            else if(t[1]=="world_count") {count(3);pass=model.world_items().size()==number<std::size_t>(t[2]);}
            else if(t[1]=="parent") {count(4);auto n=model.nodes().find(number<Id>(t[2]));pass=n!=model.nodes().end()&&n->second.parent==number<Id>(t[3]);}
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
        bool preview=false,old_session=false;
        if(t[0]=="preview") {preview=true;t.erase(t.begin());require(!t.empty(),"preview_command");}
        if(t[0]=="captured") {old_session=true;t.erase(t.begin());require(!t.empty(),"captured_command");}
        Command c;c.session=old_session?captured_session:model.session();
        std::string op=t[0];
        if(op=="spawn") {
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
        } else throw std::runtime_error("unknown_command");
        auto before=model.serialize();auto epoch=model.session();Result r{false,{}};Mass projected=-1;
        if(preview) {auto p=model.preview(c);r=p.result;projected=p.carried_mass;}
        else r=model.execute(c);
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
        require(file.eof(),"scenario_read");require(lab.begun&&lab.steps>0,"empty_scenario");
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
