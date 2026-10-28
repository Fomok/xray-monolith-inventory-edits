#pragma once
#include "model.h"
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>

namespace inventory_next {
// Persistent detached logical trees, separate from the actor-only projection.
// This is not native ownership authority: capture only an acknowledged world
// transfer, and publish this ledger and the actor change in the same save gate.
// Native item facts must be refreshed before admission; saved mass/condition
// must never overwrite the actual engine object on pickup.
class WorldSubtrees {
public:
    struct Record { Id custodian=0; Model tree; }; // 0 = ground; otherwise persistent owner ID
    const std::map<Id,Record>& records() const { return records_; }
    // Explicit stash names belong to persistent custody, not a recyclable live ID.
    // Keep this metadata on the detached world root so native item refresh cannot
    // discard it and reacquisition cannot copy it into an actor item.
    Result name_stash(Id shell,Id expected_custodian,const std::string& name) {
        const auto found=records_.find(shell);
        if(found==records_.end()||found->second.custodian!=expected_custodian)return {false,"world_storage_custodian"};
        if(!valid_stash_name(name))return {false,"world_storage_stash_name"};
        std::vector<Node> nodes;
        for(const auto& pair:found->second.tree.nodes()) {
            auto node=pair.second;
            if(node.id==2)node.attributes["stash_name"]=name;
            nodes.push_back(std::move(node));
        }
        Model tree;auto result=tree.initialize(nodes);if(!result)return result;
        auto candidate=*this;candidate.records_.at(shell).tree=std::move(tree);
        if(candidate.serialize().size()>max_bytes)return {false,"world_storage_size"};
        *this=std::move(candidate);return {true,{}};
    }
    std::string stash_name(Id shell) const {
        const auto record=records_.find(shell);if(record==records_.end())return {};
        const auto& attrs=record->second.tree.nodes().at(2).attributes;
        const auto name=attrs.find("stash_name");return name==attrs.end()?std::string{}:name->second;
    }
    Result capture(const Model& committed,Id shell,Id custodian) {
        const auto found=committed.nodes().find(shell);
        if(found==committed.nodes().end()||found->second.kind!=Kind::Item||
           !found->second.container||shell<100||(custodian&&custodian<100))return {false,"world_storage_shell"};
        Id owner=shell;
        while(committed.nodes().at(owner).parent)owner=committed.nodes().at(owner).parent;
        if(committed.nodes().at(owner).kind!=Kind::WorldRoot)return {false,"world_storage_owner"};
        Node actor;actor.id=1;actor.kind=Kind::ActorRoot;actor.category=0;actor.container=true;actor.grid={1,1};
        Node world=actor;world.id=2;world.kind=Kind::WorldRoot;world.grid={64,64};
        std::vector<Node> nodes{actor,world};
        for(const auto& pair:committed.nodes()) {
            Id id=pair.first;
            while(id&&id!=shell)id=committed.nodes().at(id).parent;
            if(!id)continue;
            auto n=pair.second;
            if(n.id==custodian)return {false,"world_storage_custodian"};
            // Only the outer placement changes when custody changes. Interior
            // positions, identities, state and nested parentage are lossless.
            if(n.id==shell){n.parent=2;n.position={};n.location=Location::Grid;n.equipped_slots=0;n.mounted_slot=0;}
            nodes.push_back(std::move(n));
        }
        Record record;record.custodian=custodian;
        auto result=record.tree.initialize(nodes);if(!result)return result;
        auto candidate=*this;
        if(!candidate.records_.emplace(shell,std::move(record)).second)return {false,"world_storage_duplicate"};
        result=candidate.validate();if(!result)return result;
        if(candidate.serialize().size()>max_bytes)return {false,"world_storage_size"};
        *this=std::move(candidate);return {true,{}};
    }
    Result adopt_empty(Node shell,Id custodian) {
        if(!shell.container||shell.kind!=Kind::Item||shell.id<100||
           (custodian&&custodian<100)||records_.count(shell.id))return {false,"world_storage_shell"};
        Node actor;actor.id=1;actor.kind=Kind::ActorRoot;actor.category=0;actor.container=true;actor.grid={1,1};
        Node world=actor;world.id=2;world.kind=Kind::WorldRoot;world.grid={64,64};
        shell.parent=2;shell.location=Location::Grid;shell.position={};
        shell.equipped_slots=0;shell.mounted_slot=0;
        Model empty;if(const auto result=empty.initialize({actor,world,shell});!result)return result;
        return capture(empty,shell.id,custodian);
    }
    Result release(Id shell,Id expected_custodian) {
        auto found=records_.find(shell);
        if(found==records_.end()||found->second.custodian!=expected_custodian)return {false,"world_storage_custodian"};
        records_.erase(found);return {true,{}};
    }
    // Candidate-only leaf edits for a direct move between a detached box and
    // another custody domain. The caller publishes the candidate only after
    // native ownership acknowledgement; loaded shells use subtree transfer.
    Result without_leaf(Id shell,Id expected_custodian,Id leaf,WorldSubtrees& output) const {
        const auto found=records_.find(shell);
        if(found==records_.end()||found->second.custodian!=expected_custodian)return {false,"world_storage_custodian"};
        const auto& saved=found->second.tree.nodes();
        if(leaf==shell||!saved.count(leaf)||saved.at(leaf).kind!=Kind::Item)return {false,"world_storage_leaf"};
        for(const auto& pair:saved)if(pair.second.parent==leaf)return {false,"world_storage_loaded"};
        std::vector<Node> nodes;for(const auto& pair:saved)if(pair.first!=leaf)nodes.push_back(pair.second);
        Model tree;auto result=tree.initialize(nodes);if(!result)return result;
        auto candidate=*this;candidate.records_.at(shell).tree=std::move(tree);
        result=candidate.validate();if(!result)return result;
        if(candidate.serialize().size()>max_bytes)return {false,"world_storage_size"};
        output=std::move(candidate);return {true,{}};
    }
    Result with_leaf(Id shell,Id expected_custodian,Node leaf,Position position,WorldSubtrees& output) const {
        const auto found=records_.find(shell);
        if(found==records_.end()||found->second.custodian!=expected_custodian)return {false,"world_storage_custodian"};
        auto candidate=*this;auto& tree=candidate.records_.at(shell).tree;
        if(tree.nodes().count(leaf.id)||tree.high_water_id()==std::numeric_limits<Id>::max())return {false,"world_storage_duplicate"};
        leaf.parent=shell;leaf.location=Location::Grid;leaf.position=position;
        leaf.equipped_slots=0;leaf.mounted_slot=0;
        Model preflight=tree;Command request;request.operation=Operation::Spawn;request.spawned=leaf;
        request.spawned.id=tree.high_water_id()+1;
        request.revision=preflight.revision();request.session=preflight.session();
        auto result=preflight.execute(request);if(!result)return result;
        std::vector<Node> nodes;for(const auto& pair:tree.nodes())nodes.push_back(pair.second);
        nodes.push_back(std::move(leaf));Model inserted;result=inserted.initialize(nodes);if(!result)return result;
        tree=std::move(inserted);
        result=candidate.validate();if(!result)return result;
        if(candidate.serialize().size()>max_bytes)return {false,"world_storage_size"};
        output=std::move(candidate);return {true,{}};
    }
    Result relocate_leaf(Id source_shell,Id source_custodian,Id leaf,Id destination_shell,
                         Id destination_custodian,Position position,WorldSubtrees& output) const {
        const auto source=records_.find(source_shell);
        if(source==records_.end()||source->second.custodian!=source_custodian||
           !source->second.tree.nodes().count(leaf)||source->second.tree.nodes().at(leaf).parent!=source_shell)
            return {false,"world_storage_source"};
        const Node item=source->second.tree.nodes().at(leaf);
        WorldSubtrees removed;
        if(const auto result=without_leaf(source_shell,source_custodian,leaf,removed);!result)return result;
        return removed.with_leaf(destination_shell,destination_custodian,item,position,output);
    }
    // Rebuild saved placement around current engine facts. Caller first checks
    // physical custody and persistent identities for every supplied object.
    Result refresh(Id shell,Id expected_custodian,const std::vector<Node>& actual) {
        const auto found=records_.find(shell);
        if(found==records_.end()||found->second.custodian!=expected_custodian)return {false,"world_storage_custodian"};
        const auto& saved=found->second.tree.nodes();
        if(actual.size()+2!=saved.size())return {false,"world_storage_incomplete"};
        std::set<Id> seen;std::vector<Node> nodes{saved.at(1),saved.at(2)};
        for(auto current:actual) {
            const auto old=saved.find(current.id);
            if(current.kind!=Kind::Item||old==saved.end()||current.id<100||!seen.insert(current.id).second)return {false,"world_storage_identity"};
            const auto previous_section=old->second.attributes.find("native_section");
            const auto section=current.attributes.find("native_section");
            if(previous_section==old->second.attributes.end()||section==current.attributes.end()||section->second!=previous_section->second)return {false,"world_storage_section"};
            current.parent=old->second.parent;current.position=old->second.position;
            current.location=old->second.location;current.equipped_slots=old->second.equipped_slots;current.mounted_slot=old->second.mounted_slot;
            nodes.push_back(std::move(current));
        }
        Model rebuilt;auto result=rebuilt.initialize(nodes);if(!result)return result;
        auto candidate=*this;candidate.records_.at(shell).tree=std::move(rebuilt);
        if(candidate.serialize().size()>max_bytes)return {false,"world_storage_size"};
        *this=std::move(candidate);return {true,{}};
    }
    std::string serialize() const {
        std::ostringstream out;out.imbue(std::locale::classic());
        out<<"SQA3_WORLD_STORAGE 1 "<<records_.size()<<'\n';
        for(const auto& pair:records_) {
            const auto bytes=pair.second.tree.serialize();
            out<<pair.first<<' '<<pair.second.custodian<<' '<<bytes.size()<<'\n'<<bytes;
        }
        const auto body=out.str();
        out<<"CRC32 "<<std::hex<<std::setw(8)<<std::setfill('0')<<crc(body)<<'\n';
        return out.str();
    }
    Result restore(const std::string& bytes) {
        if(bytes.size()>max_bytes)return {false,"world_storage_size"};
        const auto marker=bytes.rfind("CRC32 ");
        if(marker==std::string::npos||bytes.size()-marker!=15)return {false,"world_storage_checksum"};
        std::istringstream checksum(bytes.substr(marker+6));std::uint32_t expected=0;
        if(!(checksum>>std::hex>>expected)||expected!=crc(bytes.substr(0,marker)))return {false,"world_storage_checksum"};
        std::istringstream in(bytes.substr(0,marker));in.imbue(std::locale::classic());
        std::string magic;unsigned version=0;std::size_t count=0;
        if(!(in>>magic>>version>>count)||magic!="SQA3_WORLD_STORAGE"||version!=1||count>1024||in.get()!='\n')return {false,"world_storage_header"};
        WorldSubtrees candidate;
        for(std::size_t i=0;i<count;++i) {
            Id shell=0,custodian=0;std::size_t size=0;
            if(!(in>>shell>>custodian>>size)||!size||size>max_bytes||in.get()!='\n')return {false,"world_storage_record"};
            std::string payload(size,'\0');
            if(!in.read(&payload[0],static_cast<std::streamsize>(size)))return {false,"world_storage_record"};
            Record record;record.custodian=custodian;
            auto result=record.tree.deserialize(payload);if(!result)return result;
            if(!candidate.records_.emplace(shell,std::move(record)).second)return {false,"world_storage_duplicate"};
        }
        if(in.peek()!=std::char_traits<char>::eof())return {false,"world_storage_trailing"};
        auto result=candidate.validate();if(!result)return result;
        if(candidate.serialize()!=bytes)return {false,"world_storage_encoding"};
        *this=std::move(candidate);return {true,{}};
    }
private:
    static bool valid_stash_name(const std::string& name) {
        if(name.empty()||name.size()>240)return false;
        for(unsigned char c:name)if(c<32||c==127)return false;
        return true;
    }
    static constexpr std::size_t max_bytes=2*1024*1024;
    static std::uint32_t crc(const std::string& bytes) {
        std::uint32_t value=~std::uint32_t{0};
        for(unsigned char c:bytes){value^=c;for(int i=0;i<8;++i)value=(value>>1)^(0xedb88320U&(0U-(value&1U)));}
        return ~value;
    }
    Result validate() const {
        if(records_.size()>1024)return {false,"world_storage_size"};
        std::set<Id> identities;
        for(const auto& pair:records_) {
            const auto& nodes=pair.second.tree.nodes();const auto shell=nodes.find(pair.first);
            if(shell==nodes.end()||shell->second.kind!=Kind::Item||!shell->second.container||shell->second.parent!=2||
               (pair.second.custodian&&pair.second.custodian<100))return {false,"world_storage_shell"};
            if(nodes.count(1)!=1||nodes.count(2)!=1||nodes.at(1).kind!=Kind::ActorRoot||nodes.at(2).kind!=Kind::WorldRoot)return {false,"world_storage_root"};
            const auto name=nodes.at(2).attributes.find("stash_name");
            if(name!=nodes.at(2).attributes.end()&&!valid_stash_name(name->second))return {false,"world_storage_stash_name"};
            for(const auto& node:nodes)if(node.first!=1&&node.first!=2) {
                if(node.first<100||node.second.kind!=Kind::Item||!identities.insert(node.first).second)return {false,"world_storage_identity"};
                if(identities.size()>4094)return {false,"world_storage_size"};
                Id id=node.first;
                while(id&&id!=pair.first)id=nodes.at(id).parent;
                if(id!=pair.first)return {false,"world_storage_orphan"};
                if(node.first==pair.second.custodian)return {false,"world_storage_custodian"};
            }
        }
        for(const auto& pair:records_)if(identities.count(pair.second.custodian))return {false,"world_storage_custodian"};
        return {true,{}};
    }
    std::map<Id,Record> records_;
};
}
