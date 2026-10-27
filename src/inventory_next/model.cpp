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
Result analyze(const std::map<Id, Node>& nodes, DerivedMap& result) {
    if (nodes.empty() || nodes.size() > max_nodes) return bad("node_count");
    std::map<Id, std::vector<Id>> children;
    for (const auto& [id, n] : nodes) {
        if (!id || id != n.id || n.intrinsic < 0 || n.intrinsic > max_mass ||
            n.max_contents < 0 || n.max_contents > max_mass) return bad("identity_or_mass");
        if (!size_ok(n.empty) || !size_ok(n.half) || !size_ok(n.full) ||
            n.position.x < 0 || n.position.y < 0 || n.position.x > 64 || n.position.y > 64)
            return bad("dimensions");
        if (n.kind == Kind::Item) {
            auto p = nodes.find(n.parent);
            if (p == nodes.end() || !p->second.container) return bad("parent");
            if (!n.category || (n.category & (n.category - 1))) return bad("category");
            if (!(p->second.accepts & n.category)) return bad("restriction");
            children[n.parent].push_back(id);
        } else if (n.kind == Kind::ActorRoot || n.kind == Kind::WorldRoot) {
            if (n.parent || !n.container || n.soft || n.intrinsic || n.category ||
                n.position.x || n.position.y || n.position.rotated) return bad("root");
        } else return bad("kind");
        if (n.container) {
            if (!size_ok(n.grid)) return bad("grid");
        } else if (n.soft || n.grid.w || n.grid.h || n.max_contents) return bad("not_container");
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
        Derived d{n.empty, n.intrinsic, 1};
        int used = 0;
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
            Size s = rotated(cd.size, c.position.rotated);
            const Position& p = c.position;
            if (p.x + s.w > n.grid.w || p.y + s.h > n.grid.h) return bad("bounds");
            for (const auto& [q, t] : rectangles) {
                if (p.x < q.x + t.w && q.x < p.x + s.w &&
                    p.y < q.y + t.h && q.y < p.y + s.h) return bad("overlap");
            }
            rectangles.push_back({p, s});
            used += s.w * s.h;
        }
        if (n.max_contents && d.mass - n.intrinsic > n.max_contents) return bad("mass_capacity");
        if (n.soft && used) d.size = used * 2 <= n.grid.w * n.grid.h ? n.half : n.full;
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
            break;
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
std::string Model::serialize() const {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << "INVENTORY_NEXT 2 " << revision_ << ' ' << nodes_.size() << ' ' << high_water_id_ << '\n';
    for (const auto& [id, n] : nodes_) {
        s << id << ' ' << n.parent << ' ' << static_cast<int>(n.kind) << ' ' << n.intrinsic << ' '
          << n.category << ' ' << n.position.x << ' ' << n.position.y << ' ' << n.position.rotated << ' '
          << n.empty.w << ' ' << n.empty.h << ' ' << n.half.w << ' ' << n.half.h << ' '
          << n.full.w << ' ' << n.full.h << ' ' << n.container << ' ' << n.soft << ' '
          << n.grid.w << ' ' << n.grid.h << ' ' << n.accepts << ' ' << n.max_contents << '\n';
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
    if (!(s >> magic >> version >> revision >> count >> high_water) || magic != "INVENTORY_NEXT" || version != 2 ||
        count == 0 || count > max_nodes) return bad("header");
    std::vector<Node> nodes;
    for (std::size_t i = 0; i < count; ++i) {
        Node n;
        unsigned kind, rotation, container, soft;
        if (!(s >> n.id >> n.parent >> kind >> n.intrinsic >> n.category >> n.position.x >> n.position.y
            >> rotation >> n.empty.w >> n.empty.h >> n.half.w >> n.half.h >> n.full.w >> n.full.h
            >> container >> soft >> n.grid.w >> n.grid.h >> n.accepts >> n.max_contents) ||
            kind > 2 || rotation > 1 || container > 1 || soft > 1) return bad("record");
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
