#pragma once
#include "model.h"
#include <filesystem>
namespace inventory_next {
// Windows lab persistence: flush complete temp file, then replace target on same
// volume with write-through rename. Single writer. Engine save integration pending.
Result save_atomic(const Model& model, const std::filesystem::path& path);
Result load_file(Model& model, const std::filesystem::path& path);
}
