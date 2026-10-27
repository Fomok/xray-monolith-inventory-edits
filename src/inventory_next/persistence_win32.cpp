#include "persistence.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <atomic>
#include <fstream>

namespace inventory_next {
Result save_atomic(const Model& model, const std::filesystem::path& path) {
    // Validate even a default-constructed/uninitialized model before writing.
    const std::string bytes = model.serialize();
    Model verify;
    auto valid = verify.deserialize(bytes);
    if (!valid) return valid;
    static std::atomic<unsigned long> serial{0};
    auto temp = path;
    temp += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(++serial);
    HANDLE file = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return {false, "temp_open"};
    DWORD written = 0;
    bool ok = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) &&
              written == bytes.size() && FlushFileBuffers(file);
    if (!CloseHandle(file)) ok = false;
    if (!ok) { DeleteFileW(temp.c_str()); return {false, "write_flush"}; }
    if (!MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temp.c_str());
        return {false, "replace"};
    }
    return {true, {}};
}
Result load_file(Model& model, const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return {false, "open"};
    auto size = file.tellg();
    if (size < 0 || size > 2 * 1024 * 1024) return {false, "file_size"};
    std::string bytes(static_cast<std::size_t>(size), '\0');
    file.seekg(0);
    if (!file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))) return {false, "read"};
    return model.deserialize(bytes);
}
}
