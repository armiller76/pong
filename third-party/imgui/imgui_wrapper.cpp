#include "imgui_wrapper.h"

#include <string_view>

#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_renderer.h"

namespace pong
{

ImguiWrapper::ImguiWrapper(
    HWND hwnd,
    const VulkanInstance &instance,
    const VulkanDevice &device,
    VulkanRenderer &renderer,
    std::string_view project_root)
    : io{[]()
         {
             IMGUI_CHECKVERSION();
             ::ImGui::CreateContext();
             return &::ImGui::GetIO();
         }()}
    , windows_handle_{hwnd}
    , instance_{instance}
    , device_{device}
    , renderer_{renderer}
    , ini_file_{std::string(std::string(project_root) + "/third-party/imgui/imgui.ini")}
{
    io->IniFilename = ini_file_.c_str();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    startup();
}

ImguiWrapper::~ImguiWrapper()
{
    shutdown();
}

auto ImguiWrapper::startup() -> void
{
    init_windows();
    init_vulkan();
}

auto ImguiWrapper::init_windows() -> void
{
    ::ImGui_ImplWin32_Init(windows_handle_);
}

auto ImguiWrapper::init_vulkan() -> void
{
    auto init_info = ImGui_ImplVulkan_InitInfo{};
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = *instance_.native_handle();
    init_info.PhysicalDevice = *device_.physical_device();
    init_info.Device = *device_.native_handle();
    init_info.QueueFamily = device_.graphics_queue_family_index();
    init_info.Queue = device_.graphics_queue();
    // init_info.DescriptorPool = vk_renderer_.descriptor_pool_.native_handle();
    init_info.DescriptorPoolSize = 32; // == 0 -> use DescriptorPool; != 0 -> imgui creates its own
    init_info.MinImageCount = renderer_.swapchain_image_count();
    init_info.ImageCount = renderer_.swapchain_image_count();

    // TODO if you implement a pipeline cache, set this
    //  init_info.PipelineCache = ;

    // set up dynamic rendering
    auto color_attachment_formats = std::array{
        renderer_.swapchain_format(),
    };
    auto pipeline_rendering_create_info = ::vk::PipelineRenderingCreateInfo{};
    pipeline_rendering_create_info.sType = ::vk::StructureType::ePipelineRenderingCreateInfoKHR;
    pipeline_rendering_create_info.colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats.size());
    pipeline_rendering_create_info.pColorAttachmentFormats = color_attachment_formats.data();
    auto pipeline_info = ImGui_ImplVulkan_PipelineInfo{};
    pipeline_info.PipelineRenderingCreateInfo = pipeline_rendering_create_info;
    init_info.PipelineInfoMain = pipeline_info;
    init_info.UseDynamicRendering = true;

    // set up error handling
    init_info.CheckVkResultFn = check_vk_result;

    ::ImGui_ImplVulkan_Init(&init_info);
    ::ImGui::StyleColorsDark();
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

auto ImguiWrapper::recreate() -> void
{
    ::ImGui_ImplVulkan_Shutdown();
    init_vulkan();
}

auto ImguiWrapper::begin_frame() -> void
{
    ::ImGui_ImplVulkan_NewFrame();
    ::ImGui_ImplWin32_NewFrame();
    ::ImGui::NewFrame();
}

auto ImguiWrapper::render() -> void
{
    ::ImGui::Render();
    draw_data_ = ::ImGui::GetDrawData();
}

auto ImguiWrapper::get_draw_data() -> ImDrawData *
{
    return draw_data_;
}

}
