#include "file.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>

#include "utils/error.h"

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

auto File::validate_spirv() const noexcept -> bool
{
    if (data_.size() < sizeof(std::uint32_t))
    {
        return false;
    }

    if (!validate_alignment(sizeof(std::uint32_t)))
    {
        return false;
    }

    constexpr auto spirv_magic = std::uint32_t{0x07230203};
    auto magic = *reinterpret_cast<const std::uint32_t *>(data_.data());
    if (magic != spirv_magic)
    {
        return false;
    }

    arm::log::debug("valid SPIR-V: {}", path_.string());
    return true;
}

auto File::as_spirv() const -> std::span<const std::uint32_t>
{
    return data_as<std::uint32_t>();
}

auto File::read_binary(const std::filesystem::path &path) -> File
{
    return File(path);
}

auto File::read_shader(const std::filesystem::path &path) -> File
{
    auto file = File(path);
    arm::ensure(file.validate_spirv(), "loaded invalid shader: {}", path.string());
    return file;
}

auto File::exists(const std::filesystem::path &path) noexcept -> bool
{
    return std::filesystem::exists(path);
}

auto File::load_from_disk() -> void
{
    arm::ensure(exists(path_), "file does not exist: {}", path_.string());

    auto file = std::ifstream(path_.string(), std::ios::binary | std::ios::ate);
    arm::ensure(file.is_open(), "failed to open file: {}", path_.string());

    auto file_size = static_cast<std::size_t>(file.tellg());
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

auto File::validate_alignment(std::size_t alignment) const noexcept -> bool
{
    arm::ensure(alignment > 0, "invalid alignment");
    if (data_.size() % alignment != 0)
    {
        arm::log::debug("data not aligned. file={} size={}, alignment={}", path_.string(), data_.size(), alignment);
        return false;
    }
    return true;
}

} // namespace pong
