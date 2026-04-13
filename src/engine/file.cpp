#include "file.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>

#include "utils/error.h"
#include "utils/exception.h"

namespace pong
{

File::File(const std::filesystem::path &path)
    : path_(path)
{
    load_from_disk();
}

auto File::data() const -> std::span<const std::byte>
{
    return data_;
}

auto File::size() const -> std::size_t
{
    return data_.size();
}

auto File::path() const -> const std::filesystem::path &
{
    return path_;
}

auto File::empty() const -> bool
{
    return data_.empty();
}

auto File::exists(const std::filesystem::path &path) -> bool
{
    return std::filesystem::exists(path);
}

auto File::load_from_disk() -> void
{
    arm::ensure(exists(path_), "file does not exist: {}", path_.string());

    auto file = std::ifstream(path_, std::ios::binary | std::ios::ate);
    arm::ensure(file.is_open(), "failed to open file: {}", path_.string());

    auto streampos = file.tellg();
    if (!file || streampos == static_cast<std::streampos>(-1))
    { // tellg failed
        throw arm::Exception("unknown file error");
    }

    auto file_size = static_cast<std::size_t>(streampos);
    if (file_size == 0)
    {
        arm::log::warn("loaded empty file: {}", path_.string());
    }
    data_.resize(file_size);

    file.seekg(0);
    file.read(reinterpret_cast<char *>(data_.data()), file_size);

    arm::ensure(
        !!file && file.gcount() == static_cast<std::streamsize>(file_size),
        "read failure: file={} expected={}, read={}",
        path_.string(),
        file_size,
        file.gcount());
    arm::log::debug("Loaded file: {} ({} bytes)", path_.string(), file_size);
}

} // namespace pong
