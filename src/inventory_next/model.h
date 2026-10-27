#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace inventory_next {
using Id = std::uint64_t;
using Mass = std::int64_t; // integer grams, intrinsic mass excludes child item IDs
struct Size { int w = 1, h = 1; };
struct Position { int x = 0, y = 0; bool rotated = false; };
enum class Kind { Item, ActorRoot, WorldRoot };
struct Node {
    Id id = 0, parent = 0;
    Kind kind = Kind::Item;
    Mass intrinsic = 0;
    std::uint64_t category = 1;
    Position position;
    Size empty, half, full;
    bool container = false, soft = false;
    Size grid{0, 0};
    std::uint64_t accepts = ~std::uint64_t{0};
    Mass max_contents = 0; // zero means unlimited, recursive mass including containers
};
struct Result {
    bool ok;
    std::string error;
    explicit operator bool() const { return ok; }
};
struct Move { Id item, destination; Position position; std::uint64_t revision; std::uint64_t session = 0; };
enum class Operation { Move, Spawn, Erase, SetMass };
struct Command {
    Operation operation = Operation::Move;
    Id item = 0, destination = 0;
    Position position;
    Mass mass = 0;
    Node spawned;
    std::uint64_t revision = 0;
    std::uint64_t session = 0;
};
struct Preview {
    Result result;
    Mass carried_mass = 0;
    std::size_t node_count = 0;
    std::uint64_t revision = 0;
};

// Single-threaded value model. UI never gets mutable node access. Each transaction
// validates the complete candidate before publishing; no partial resize/mass edits.
class Model {
public:
    Result initialize(const std::vector<Node>& nodes);
    Result move(const Move& request);
    Result execute(const Command& request);
    Preview preview(const Command& request) const;
    const std::map<Id, Node>& nodes() const { return nodes_; }
    std::uint64_t revision() const { return revision_; }
    Id high_water_id() const { return high_water_id_; }
    std::uint64_t session() const { return session_; }
    Size footprint(Id id) const;
    Mass subtree_mass(Id id) const;
    Mass carried_mass() const;
    // Already observed world objects, independent of open panels, no pickup side effect.
    // Engine adapter must filter distance/LOS/access and recheck it on transfer.
    std::vector<Id> world_items() const;
    std::string serialize() const;
    Result deserialize(const std::string& bytes);
private:
    std::map<Id, Node> nodes_;
    std::uint64_t revision_ = 0;
    Id high_water_id_ = 0;
    std::uint64_t session_ = 0; // process-local epoch, never serialized
};
} // namespace inventory_next
