#include "model.h"
#include <algorithm>
#include <atomic>
#include <functional>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace inventory_next {
namespace {
constexpr std::size_t max_nodes = 4096;
constexpr Mass max_mass = 1000000000000LL;
constexpr std::size_t max_save = 2 * 1024 * 1024;
std::atomic<std::uint64_t> session_sequence{0};
struct Derived { Size size; Mass mass = 0; int depth = 0; };
using DerivedMap = std::map<Id, Derived>;
Result good() { return {true, {}}; }
Result bad(const std::string& why) { return {false, why}; }
bool size_ok(Size s) { return s.w > 0 && s.h > 0 && s.w <= 64 && s.h <= 64; }
Size rotated(Size s, bool r) { return r ? Size{s.h, s.w} : s; }
unsigned bits(std::uint64_t value) {
    unsigned count = 0;
    while (value) { value &= value - 1; ++count; }
    return count;
}
bool stack_compatible(const Node& a, const Node& b) {
    return !a.container && !b.container && !a.wear_slots && !b.wear_slots &&
        a.stack_limit > 1 && a.type && a.type == b.type && a.state == b.state &&
        a.intrinsic == b.intrinsic && a.category == b.category && a.stack_limit == b.stack_limit &&
        a.empty.w == b.empty.w && a.empty.h == b.empty.h;
}
unsigned region_of(const Node& n, int x, int y) {
    if (!n.container || x<0 || y<0 || x>=n.grid.w || y>=n.grid.h) return 0;
    return n.cells.empty() ? 1 : n.cells[static_cast<std::size_t>(y*n.grid.w+x)];
}
Result analyze(const std::map<Id, Node>& nodes, DerivedMap& result) {
    if (nodes.empty() || nodes.size() > max_nodes) return bad("node_count");
    std::map<Id, std::vector<Id>> children;
    std::size_t authored_cells = 0;
    for (const auto& [id, n] : nodes) {
        if (!id || id != n.id || n.intrinsic < 0 || n.intrinsic > max_mass ||
            n.max_contents < 0 || n.max_contents > max_mass) return bad("identity_or_mass");
        if (!size_ok(n.empty) || !size_ok(n.half) || !size_ok(n.full) ||
            n.position.x < 0 || n.position.y < 0 || n.position.x > 64 || n.position.y > 64)
            return bad("dimensions");
        if (!n.quantity || !n.stack_limit || n.stack_limit > 1000000 || n.quantity > n.stack_limit ||
            (n.stack_limit > 1 && (!n.type || n.container || n.wear_slots)) ||
            n.intrinsic > max_mass / n.quantity) return bad("stack_definition");
        if (n.location != Location::Grid && n.location != Location::Equipment) return bad("location");
        if (n.required_slots == 0 || n.required_slots > 64) return bad("slot_count");
        if (n.kind != Kind::ActorRoot && (n.allowed_slots || n.equipment_accepts || n.equipment_capacity))
            return bad("equipment_policy");
        if (n.kind == Kind::Item) {
            auto p = nodes.find(n.parent);
            if (p == nodes.end() || !p->second.container) return bad("parent");
            if (!n.category || (n.category & (n.category - 1))) return bad("category");
            if (!(p->second.accepts & n.category)) return bad("restriction");
            if (n.location == Location::Equipment) {
                const Node& owner = p->second;
                if (owner.kind != Kind::ActorRoot || n.quantity != 1 || !n.wear_slots ||
                    !n.equipped_slots || (n.equipped_slots & ~n.wear_slots) ||
                    (n.equipped_slots & ~owner.allowed_slots) || !(owner.equipment_accepts & n.category) ||
                    bits(n.equipped_slots) != n.required_slots ||
                    n.position.x || n.position.y || n.position.rotated) return bad("equipment");
            } else if (n.equipped_slots) return bad("equipment");
            children[n.parent].push_back(id);
        } else if (n.kind == Kind::ActorRoot || n.kind == Kind::WorldRoot) {
            if (n.parent || !n.container || n.soft || n.intrinsic || n.category ||
                n.position.x || n.position.y || n.position.rotated || n.location != Location::Grid ||
                n.equipped_slots || n.wear_slots || n.quantity != 1 || n.stack_limit != 1) return bad("root");
        } else return bad("kind");
        if (n.container) {
            if (!size_ok(n.grid)) return bad("grid");
            if (!n.cells.empty()) {
                if (n.cells.size()!=static_cast<std::size_t>(n.grid.w*n.grid.h)) return bad("cell_definition");
                unsigned usable=0;
                for (unsigned region:n.cells) { if (region>4096) return bad("cell_definition"); if (region) ++usable; }
                if (!usable) return bad("cell_definition");
                authored_cells+=n.cells.size();
                if (authored_cells>65536) return bad("cell_budget");
            }
        } else if (n.soft || n.grid.w || n.grid.h || n.max_contents || !n.cells.empty()) return bad("not_container");
        if (n.soft && (n.half.w < n.empty.w || n.half.h < n.empty.h ||
            n.full.w < n.half.w || n.full.h < n.half.h ||
            n.full.w * n.full.h < n.grid.w * n.grid.h)) return bad("growth_definition");
    }
    std::map<Id, int> color;
    std::function<Result(Id, int)> visit = [&](Id id, int call_depth) -> Result {
        if (call_depth > 64) return bad("depth");
        if (color[id] == 1) return bad("cycle");
        if (color[id] == 2) return good();
        color[id] = 1;
        const Node& n = nodes.at(id);
        Derived d{n.empty, n.intrinsic * n.quantity, 1};
        int used = 0;
        std::vector<const Node*> equipped;
        std::vector<std::pair<Position, Size>> rectangles;
        for (Id child : children[id]) {
            auto check = visit(child, call_depth + 1);
            if (!check) return check;
            const Node& c = nodes.at(child);
            const auto& cd = result.at(child);
            if (d.mass > max_mass - cd.mass) return bad("mass_overflow");
            d.mass += cd.mass;
            d.depth = std::max(d.depth, cd.depth + 1);
            if (d.depth > 64) return bad("depth");
            if (c.location == Location::Equipment) {
                for (const Node* other : equipped) {
                    if ((c.equipped_slots & other->equipped_slots) ||
                        (c.blocked_slots & other->equipped_slots) ||
                        (other->blocked_slots & c.equipped_slots)) return bad("equipment_conflict");
                }
                equipped.push_back(&c);
                if (equipped.size() > n.equipment_capacity) return bad("equipment_capacity");
                continue; // still owned and weighed, but occupies no grid cells
            }
            Size s = rotated(cd.size, c.position.rotated);
            const Position& p = c.position;
            if (p.x + s.w > n.grid.w || p.y + s.h > n.grid.h) return bad("bounds");
            const unsigned region=region_of(n,p.x,p.y);
            for (int y=0;y<s.h;++y) for (int x=0;x<s.w;++x) {
                const auto cell=region_of(n,p.x+x,p.y+y);
                if (!cell) return bad("blocked_cell");
                if (cell!=region) return bad("compartment");
            }
            for (const auto& [q, t] : rectangles) {
                if (p.x < q.x + t.w && q.x < p.x + s.w &&
                    p.y < q.y + t.h && q.y < p.y + s.h) return bad("overlap");
            }
            rectangles.push_back({p, s});
            used += s.w * s.h;
        }
        if (n.max_contents && d.mass - n.intrinsic > n.max_contents) return bad("mass_capacity");
        int capacity=n.grid.w*n.grid.h;
        if (!n.cells.empty()) capacity=static_cast<int>(std::count_if(n.cells.begin(),n.cells.end(),[](unsigned c){return c!=0;}));
        if (n.soft && used) d.size = used * 2 <= capacity ? n.half : n.full;
        result[id] = d;
        color[id] = 2;
        return good();
    };
    // Visit even components disconnected from roots: cycles cannot hide in them.
    for (const auto& [id, n] : nodes) { (void)n; auto r = visit(id, 1); if (!r) return r; }
    Mass forest_mass = 0;
    for (const auto& [id, n] : nodes) if (n.kind != Kind::Item) {
        if (forest_mass > max_mass - result.at(id).mass) return bad("mass_overflow");
        forest_mass += result.at(id).mass;
    }
    return good();
}
std::uint32_t crc32(const std::string& s) {
    std::uint32_t crc = ~std::uint32_t{0};
    for (unsigned char c : s) {
        crc ^= c;
        for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1U)));
    }
    return ~crc;
}
} // namespace

Result Model::initialize(const std::vector<Node>& nodes) {
    std::map<Id, Node> candidate;
    for (const Node& n : nodes) if (!candidate.emplace(n.id, n).second) return bad("duplicate_id");
    DerivedMap derived;
    auto r = analyze(candidate, derived);
    if (!r) return r;
    nodes_ = std::move(candidate);
    revision_ = 0;
    high_water_id_ = nodes_.rbegin()->first;
    session_ = ++session_sequence;
    return good();
}
Result Model::move(const Move& request) {
    Command c;
    c.item = request.item;
    c.destination = request.destination;
    c.position = request.position;
    c.revision = request.revision;
    c.session = request.session;
    return execute(c);
}
Preview Model::preview(const Command& request) const {
    Model candidate = *this;
    auto r = candidate.execute(request);
    if (!r) return {r, carried_mass(), nodes_.size(), revision_};
    return {r, candidate.carried_mass(), candidate.nodes_.size(), candidate.revision_};
}
Result Model::execute_batch(const std::vector<Command>& requests) {
    if (requests.empty() || requests.size() > 64) return bad("batch_size");
    Model candidate = *this;
    for (auto request : requests) {
        if (request.session != session_) return bad("stale_session");
        if (request.revision != revision_) return bad("stale_revision");
        request.revision = candidate.revision_;
        auto r = candidate.execute(request);
        if (!r) return r;
    }
    // Each ordered stage is legal; publish the whole command list once. A failed
    // stage discards all creations/deletions/IDs. This is not a simultaneous swap.
    candidate.revision_ = revision_ + 1;
    *this = std::move(candidate);
    return good();
}
Result Model::relocate(const std::vector<Move>& requests) {
    if (requests.empty() || requests.size() > 64) return bad("batch_size");
    if (revision_ == std::numeric_limits<std::uint64_t>::max()) return bad("revision_overflow");
    auto candidate = nodes_;
    std::map<Id, bool> targeted;
    for (const auto& request : requests) {
        if (!session_ || request.session != session_) return bad("stale_session");
        if (request.revision != revision_) return bad("stale_revision");
        if (!targeted.emplace(request.item, true).second) return bad("duplicate_target");
        auto item = candidate.find(request.item);
        if (item == candidate.end() || item->second.kind != Kind::Item) return bad("item");
        item->second.parent = request.destination;
        item->second.position = request.position;
        item->second.location = Location::Grid;
        item->second.equipped_slots = 0;
    }
    DerivedMap derived;
    auto r = analyze(candidate, derived);
    if (!r) return r;
    nodes_.swap(candidate);
    ++revision_;
    return good();
}
Result Model::execute(const Command& request) {
    if (!session_ || request.session != session_) return bad("stale_session");
    if (request.revision != revision_) return bad("stale_revision");
    if (revision_ == std::numeric_limits<std::uint64_t>::max()) return bad("revision_overflow");
    auto candidate = nodes_;
    Id next_high_water = high_water_id_;
    if (request.operation == Operation::Spawn) {
        if (request.spawned.kind != Kind::Item) return bad("spawn_root");
        if (request.spawned.id <= high_water_id_) return bad("id_reuse");
        candidate.emplace(request.spawned.id, request.spawned);
        next_high_water = request.spawned.id;
    } else {
        auto item = candidate.find(request.item);
        if (item == candidate.end() || item->second.kind != Kind::Item) return bad("item");
        switch (request.operation) {
        case Operation::Move:
            item->second.parent = request.destination;
            item->second.position = request.position;
            item->second.location = Location::Grid;
            item->second.equipped_slots = 0;
            break;
        case Operation::Equip:
            item->second.parent = request.destination;
            item->second.position = {};
            item->second.location = Location::Equipment;
            item->second.equipped_slots = request.slots;
            break;
        case Operation::Split: {
            Node& source = item->second;
            if (!stack_compatible(source, source) || !request.quantity || request.quantity >= source.quantity)
                return bad("split_quantity");
            if (request.new_id <= high_water_id_) return bad("id_reuse");
            Node split = source;
            split.id = request.new_id;
            split.parent = request.destination;
            split.position = request.position;
            split.quantity = request.quantity;
            source.quantity -= request.quantity;
            candidate.emplace(split.id, split);
            next_high_water = split.id;
            break;
        }
        case Operation::Merge: {
            auto target = candidate.find(request.destination);
            if (target == candidate.end() || target == item || !stack_compatible(item->second, target->second))
                return bad("stack_compatibility");
            if (!request.quantity || request.quantity > item->second.quantity ||
                request.quantity > target->second.stack_limit - target->second.quantity) return bad("merge_quantity");
            target->second.quantity += request.quantity;
            item->second.quantity -= request.quantity;
            if (!item->second.quantity) candidate.erase(item);
            break;
        }
        case Operation::Erase:
            for (const auto& [id, n] : candidate) {
                (void)id;
                if (n.parent == request.item) return bad("not_empty");
            }
            candidate.erase(item);
            break;
        case Operation::SetMass: item->second.intrinsic = request.mass; break;
        default: return bad("operation");
        }
    }
    DerivedMap derived;
    auto r = analyze(candidate, derived);
    if (!r) return r;
    nodes_.swap(candidate);
    high_water_id_ = next_high_water;
    ++revision_;
    return good();
}
Size Model::footprint(Id id) const {
    DerivedMap d;
    if (!analyze(nodes_, d) || !d.count(id)) return {0, 0};
    return d.at(id).size;
}
Mass Model::subtree_mass(Id id) const {
    DerivedMap d;
    if (!analyze(nodes_, d) || !d.count(id)) return 0;
    return d.at(id).mass;
}
Mass Model::carried_mass() const {
    DerivedMap d;
    if (!analyze(nodes_, d)) return 0;
    Mass sum = 0;
    for (const auto& [id, n] : nodes_) if (n.kind == Kind::ActorRoot) sum += d.at(id).mass;
    return sum;
}
std::vector<Id> Model::world_items() const {
    std::vector<Id> visible;
    for (const auto& [id, n] : nodes_) if (n.kind == Kind::Item &&
        nodes_.at(n.parent).kind == Kind::WorldRoot) visible.push_back(id);
    return visible;
}
std::vector<ItemView> Model::snapshot() const {
    DerivedMap d;
    std::vector<ItemView> view;
    if (!analyze(nodes_, d)) return view;
    for (const auto& [id, n] : nodes_) if (n.kind == Kind::Item) {
        view.push_back({id, n.parent, n.type, n.state, n.location, n.position,
            rotated(d.at(id).size, n.position.rotated), d.at(id).mass,
            n.quantity, n.equipped_slots, n.presentation_group});
    }
    return view;
}
unsigned Model::cell_region(Id container, int x, int y) const {
    auto n=nodes_.find(container);
    return n==nodes_.end() ? 0 : region_of(n->second,x,y);
}
std::vector<Id> Model::equipment_group(unsigned group) const {
    std::vector<Id> ids;
    for (const auto& [id, n] : nodes_)
        if (n.kind == Kind::Item && n.location == Location::Equipment && n.presentation_group == group) ids.push_back(id);
    return ids;
}
Id Model::cycle_equipment(unsigned group, Id selected, int direction) const {
    auto ids = equipment_group(group);
    if (ids.empty()) return 0;
    auto it = std::find(ids.begin(), ids.end(), selected);
    if (it == ids.end()) return ids.front();
    auto index = static_cast<std::size_t>(it - ids.begin());
    return direction < 0 ? ids[(index + ids.size() - 1) % ids.size()] : ids[(index + 1) % ids.size()];
}
std::string Model::serialize() const {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << "INVENTORY_NEXT 4 " << revision_ << ' ' << nodes_.size() << ' ' << high_water_id_ << '\n';
    for (const auto& [id, n] : nodes_) {
        s << id << ' ' << n.parent << ' ' << static_cast<int>(n.kind) << ' ' << n.intrinsic << ' '
          << n.category << ' ' << n.position.x << ' ' << n.position.y << ' ' << n.position.rotated << ' '
          << n.empty.w << ' ' << n.empty.h << ' ' << n.half.w << ' ' << n.half.h << ' '
          << n.full.w << ' ' << n.full.h << ' ' << n.container << ' ' << n.soft << ' '
          << n.grid.w << ' ' << n.grid.h << ' ' << n.accepts << ' ' << n.max_contents << ' '
          << n.type << ' ' << n.state << ' ' << n.quantity << ' ' << n.stack_limit << ' '
          << static_cast<unsigned>(n.location) << ' ' << n.wear_slots << ' ' << n.blocked_slots << ' '
          << n.equipped_slots << ' ' << n.required_slots << ' ' << n.presentation_group << ' '
          << n.allowed_slots << ' ' << n.equipment_accepts << ' ' << n.equipment_capacity << ' ' << n.cells.size();
        for (unsigned cell:n.cells) s << ' ' << cell;
        s << '\n';
    }
    std::string body = s.str();
    s << "CRC32 " << std::hex << std::setw(8) << std::setfill('0') << crc32(body) << '\n';
    return s.str();
}
Result Model::deserialize(const std::string& bytes) {
    if (bytes.size() > max_save || bytes.find('-') != std::string::npos) return bad("save_size_or_sign");
    auto at = bytes.rfind("CRC32 ");
    if (at == std::string::npos) return bad("checksum_missing");
    std::string body = bytes.substr(0, at);
    std::istringstream tail(bytes.substr(at + 6));
    tail.imbue(std::locale::classic());
    std::uint32_t crc;
    if (!(tail >> std::hex >> crc) || !(tail >> std::ws).eof() || crc != crc32(body)) return bad("checksum");
    std::istringstream s(body);
    s.imbue(std::locale::classic());
    std::string magic;
    unsigned version;
    std::uint64_t revision;
    std::size_t count;
    Id high_water;
    if (!(s >> magic >> version >> revision >> count >> high_water) || magic != "INVENTORY_NEXT" || version != 4 ||
        count == 0 || count > max_nodes) return bad("header");
    std::vector<Node> nodes;
    for (std::size_t i = 0; i < count; ++i) {
        Node n;
        unsigned kind, rotation, container, soft, location;
        if (!(s >> n.id >> n.parent >> kind >> n.intrinsic >> n.category >> n.position.x >> n.position.y
            >> rotation >> n.empty.w >> n.empty.h >> n.half.w >> n.half.h >> n.full.w >> n.full.h
            >> container >> soft >> n.grid.w >> n.grid.h >> n.accepts >> n.max_contents
            >> n.type >> n.state >> n.quantity >> n.stack_limit >> location >> n.wear_slots >> n.blocked_slots
            >> n.equipped_slots >> n.required_slots >> n.presentation_group >> n.allowed_slots
            >> n.equipment_accepts >> n.equipment_capacity) ||
            kind > 2 || rotation > 1 || container > 1 || soft > 1 || location > 1) return bad("record");
        n.location = static_cast<Location>(location);
        std::size_t cell_count;
        if (!(s >> cell_count) || cell_count>4096) return bad("cell_definition");
        n.cells.resize(cell_count);
        for (auto& cell:n.cells) if (!(s >> cell)) return bad("cell_definition");
        n.kind = static_cast<Kind>(kind);
        n.position.rotated = rotation != 0;
        n.container = container != 0;
        n.soft = soft != 0;
        nodes.push_back(n);
    }
    if (!(s >> std::ws).eof()) return bad("trailing_data");
    Model candidate;
    auto r = candidate.initialize(nodes);
    if (!r) return r;
    if (high_water < candidate.high_water_id_) return bad("high_water");
    candidate.high_water_id_ = high_water;
    candidate.revision_ = revision;
    *this = std::move(candidate);
    return good();
}
} // namespace inventory_next
