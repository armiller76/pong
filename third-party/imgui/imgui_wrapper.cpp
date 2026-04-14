#include "imgui_wrapper.h"

#include <functional>
#include <string_view>
#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include "engine/vulkan/vulkan_descriptor_pool.h"
#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_renderer.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/error.h"

namespace pong
{

ImguiWrapper::ImguiWrapper(
    HWND hwnd,
    VulkanRenderer &renderer,
    const VulkanInstance &instance,
    std::string_view project_root)
    : io{[]()
         {
             IMGUI_CHECKVERSION();
             ::ImGui::CreateContext();
             return &::ImGui::GetIO();
         }()}
    , windows_handle_{hwnd}
    , vk_renderer_{renderer}
    , vk_instance_{instance}
    , ini_file_{std::move(std::string(std::string(project_root) + "/third-party/imgui/imgui.ini"))}
{
    vk_renderer_.set_imgui_resize_callback_([this]() { this->framebuffer_resize_callback(); });
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
    init_info.Instance = *vk_instance_.get();
    init_info.PhysicalDevice = *vk_renderer_.device_.physical_device();
    init_info.Device = *vk_renderer_.device_.native_handle();
    init_info.QueueFamily = vk_renderer_.device_.graphics_queue_family_index();
    init_info.Queue = vk_renderer_.device_.graphics_queue();
    // init_info.DescriptorPool = vk_renderer_.descriptor_pool_.native_handle();
    init_info.DescriptorPoolSize = 32; // == 0 -> use DescriptorPool; != 0 -> imgui creates its own
    init_info.MinImageCount = vk_renderer_.swapchain_.image_count();
    init_info.ImageCount = vk_renderer_.swapchain_.image_count();

    // TODO if you implement a pipeline cache, set this
    //  init_info.PipelineCache = ;

    // set up dynamic rendering
    auto pipeline_rendering_create_info = ::vk::PipelineRenderingCreateInfo{};
    pipeline_rendering_create_info.sType = ::vk::StructureType::ePipelineRenderingCreateInfoKHR;
    pipeline_rendering_create_info.colorAttachmentCount = 1;
    pipeline_rendering_create_info.pColorAttachmentFormats = &vk_renderer_.swapchain_.format();
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
    vk_renderer_.device_.native_handle().waitIdle();
    vk_renderer_.set_imgui_resize_callback_(nullptr);
    ::ImGui_ImplVulkan_Shutdown();
    ::ImGui_ImplWin32_Shutdown();
    ::ImGui::DestroyContext();
}

void ImguiWrapper::framebuffer_resize_callback()
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
