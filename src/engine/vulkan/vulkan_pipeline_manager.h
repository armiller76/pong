#pragma once

#include <unordered_map>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_pipeline_factory.h"
#include "engine/vulkan/vulkan_pipeline_types.h"
#include "vulkan_pipeline_types.h"

namespace pong
{

class VulkanPipelineManager
{
  public:
    VulkanPipelineManager();

  private:
    VulkanPipelineFactory factory_;
    std::unordered_map<PipelineKey, PipelineEntry> pipelines_; // ?
};

}
