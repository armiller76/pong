#include "imgui_wrapper.h"

#include <format>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "engine/render_context.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/render_context.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_utils.h"

namespace pong
{

ImguiWrapper::ImguiWrapper(
    HWND hwnd,
    const VulkanInstance &instance,
    RenderContext &render_context,
    std::string_view project_root)
    : io_{[]() -> ImGuiIO *
          {
              IMGUI_CHECKVERSION();
              ::ImGui::CreateContext();
              return &::ImGui::GetIO();
          }()}
    , windows_handle_{hwnd}
    , instance_{instance}
    , render_context_{render_context}
    , ini_file_{std::format("{}{}", project_root, "/third-party/imgui/imgui.ini")}
{
    io_->IniFilename = ini_file_.c_str();
    io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    startup_();
}

ImguiWrapper::~ImguiWrapper()
{
}

auto ImguiWrapper::recreate() -> void
{
    ::ImGui_ImplWin32_Shutdown();
    ::ImGui_ImplVulkan_Shutdown();
    init_vulkan_();
    init_windows_();
}

auto ImguiWrapper::draw_fps() -> void
{
    const auto viewport = ::ImGui::GetMainViewport();
    ::ImGui::SetNextWindowPos(
        {viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f},
        ::ImGuiCond_Always,
        {1.0f, 0.0f});
    ::ImGui::SetNextWindowBgAlpha(0.35f);
    ::ImGui::Begin(
        "FPS",
        nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
    ::ImGui::Text("%.3fms/%.1fFPS", 1000.0f / io_->Framerate, io_->Framerate);
    ::ImGui::End();
}

auto ImguiWrapper::draw_settings() -> void
{
}

auto ImguiWrapper::render() -> void
{
    ::ImGui_ImplVulkan_NewFrame();
    ::ImGui_ImplWin32_NewFrame();
    ::ImGui::NewFrame();

    draw_fps();
    draw_settings();

    ::ImGui::Render();
    draw_data_ = ::ImGui::GetDrawData();
}

auto ImguiWrapper::get_draw_data() -> ImDrawData *
{
    return draw_data_;
}

auto ImguiWrapper::shutdown() -> void
{
    if (::ImGui::GetCurrentContext() != nullptr)
    {
        ::ImGui_ImplVulkan_Shutdown();
        ::ImGui_ImplWin32_Shutdown();
        ::ImGui::DestroyContext();
    }
}

auto ImguiWrapper::startup_() -> void
{
    init_vulkan_();
    init_windows_();
}

auto ImguiWrapper::init_vulkan_() -> void
{
    auto color_attachment_formats = std::array{
        render_context_.renderer().swapchain_format(),
    };
    auto pipeline_rendering_create_info = ::vk::PipelineRenderingCreateInfo{};
    pipeline_rendering_create_info.sType = ::vk::StructureType::ePipelineRenderingCreateInfoKHR;
    pipeline_rendering_create_info.pNext = nullptr;
    pipeline_rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats.size());
    pipeline_rendering_create_info.pColorAttachmentFormats = color_attachment_formats.data();
    pipeline_rendering_create_info.depthAttachmentFormat = render_context_.renderer().depth_format();
    pipeline_rendering_create_info.stencilAttachmentFormat = ::vk::Format::eUndefined;

    auto pipeline_info = ImGui_ImplVulkan_PipelineInfo{};
    pipeline_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;

    auto init_info = ImGui_ImplVulkan_InitInfo{};
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = instance_.native_handle();
    init_info.PhysicalDevice = render_context_.device().physical_device_native_handle();
    init_info.Device = render_context_.device().native_handle();
    init_info.QueueFamily = render_context_.device().graphics_queue_family_index();
    init_info.Queue = render_context_.device().graphics_queue();
    init_info.DescriptorPool = nullptr;
    init_info.DescriptorPoolSize = 128; // == 0 -> use DescriptorPool; != 0 -> imgui creates its own
    init_info.MinImageCount = render_context_.renderer().swapchain_image_count();
    init_info.ImageCount = render_context_.renderer().swapchain_image_count();
    init_info.PipelineCache = nullptr; // TODO if you implement a pipeline cache, set this
    init_info.PipelineInfoMain = pipeline_info;
    init_info.UseDynamicRendering = true;
    init_info.CheckVkResultFn = check_vk_result;
    ::ImGui_ImplVulkan_Init(&init_info);
    ::ImGui::StyleColorsDark();
}

auto ImguiWrapper::init_windows_() -> void
{
    ::ImGui_ImplWin32_Init(windows_handle_);
}

}
