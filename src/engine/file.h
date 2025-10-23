#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

#include "utils/error.h"

namespace pong
{

class File
{
  public:
    explicit File(const std::filesystem::path &path);

    File(File &&other) noexcept = default;
    File &operator=(File &&other) noexcept = default;
    File(const File &) = delete;
    File &operator=(const File &) = delete;
    ~File() = default;

    auto data() const -> std::span<const std::byte>;
    template <class T>
    auto data_as() const -> std::span<const T>;

    auto size() const -> std::size_t;
    auto path() const -> const std::filesystem::path &;
    auto empty() const -> bool;

    auto validate_spirv() const noexcept -> bool;
    auto as_spirv() const -> std::span<const uint32_t>;

    static auto read_binary(const std::filesystem::path &path) -> File;
    static auto read_shader(const std::filesystem::path &path) -> File;
    static auto exists(const std::filesystem::path &path) noexcept -> bool;

  private:
    std::filesystem::path path_;
    std::vector<std::byte> data_;

    auto load_from_disk() -> void;
    auto validate_alignment(std::size_t alignment) const noexcept -> bool;
};

template <class T>
auto File::data_as() const -> std::span<const T>
{
    arm::ensure(
        validate_alignment(alignof(T)),
        "data not aligned for type {} (alignment {}): {}",
        typeid(T).name(),
        alignof(T),
        path_.string());

    return std::span<const T>(reinterpret_cast<const T *>(data_.data()), data_.size() / sizeof(T));
}

} // namespace pong
