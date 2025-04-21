#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_vulkan.h>
#include <string>

 class VulkanHelpers
{
public:
	void SetupVulkan(std::vector<const char*>& instExtensions);
	void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
	void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
	void FramePresent(ImGui_ImplVulkanH_Window* wd);
	void CleanupVulkan();
	void CleanupVulkanWindow();
	bool UpdateUIScaling(float scale);
	static void checkVkResult(VkResult err);
public:
uint32_t  minImageCount = 2;
VkAllocationCallbacks* allocator = nullptr;
VkInstance               instance = VK_NULL_HANDLE;
VkPhysicalDevice         physDevice = VK_NULL_HANDLE;
VkDevice                 device = VK_NULL_HANDLE;
uint32_t                 queueFamily = (uint32_t)-1;
VkQueue                  queue = VK_NULL_HANDLE;
VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
VkPipelineCache          pipelineCache = VK_NULL_HANDLE;
VkDescriptorPool         descriptorPool = VK_NULL_HANDLE;
ImGui_ImplVulkanH_Window mainWindowD;
bool                     swapchainRebuild = false;
};

