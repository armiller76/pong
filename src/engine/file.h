#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

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

    auto size() const -> std::size_t;
    auto path() const -> const std::filesystem::path &;
    auto empty() const -> bool;

    static auto exists(const std::filesystem::path &path) -> bool;

  private:
    std::filesystem::path path_;
    std::vector<std::byte> data_;

    auto load_from_disk() -> void;
};

} // namespace pong
