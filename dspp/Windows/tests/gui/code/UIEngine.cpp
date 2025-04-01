#include "UIEngine.h"

UIEngine::UIEngine()
{
}

UIEngine::UIEngine(SDL_Window* window, VulkanHelpers& helper) : vulkan(helper), window(window)
{
    isReal = true;
}

UIEngine::~UIEngine()
{
    if (!isReal) return;
    for (const auto& element : elements) {
        if (element.second->IsActive()) {
            element.second->OnDestroy();
        }
    }
    VkResult err = vkDeviceWaitIdle(vulkan.device);
    vulkan.checkVkResult(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    vulkan.CleanupVulkanWindow();
    vulkan.CleanupVulkan();
        
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void UIEngine::Render()
{
    ImGui_ImplVulkanH_Window* wd = &vulkan.mainWindowD;
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Resize swap chain?
    int fb_width, fb_height;
    SDL_GetWindowSize(window, &fb_width, &fb_height);
    if (fb_width > 0 && fb_height > 0 && (vulkan.swapchainRebuild || vulkan.mainWindowD.Width != fb_width || vulkan.mainWindowD.Height != fb_height))
    {
        ImGui_ImplVulkan_SetMinImageCount(vulkan.minImageCount);
        ImGui_ImplVulkanH_CreateOrResizeWindow(vulkan.instance, vulkan.physDevice, vulkan.device, &vulkan.mainWindowD, vulkan.queueFamily, vulkan.allocator, fb_width, fb_height, vulkan.minImageCount);
        vulkan.mainWindowD.FrameIndex = 0;
        vulkan.swapchainRebuild = false;
    }

    currentSize = ImVec2(fb_width, fb_height);


    // Start the Dear ImGui frame



    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();



    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(fb_width), static_cast<float>(fb_height)));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("UIEng", nullptr, flags)) {

        // Render each GUI element in the elements vector
        for (const auto& element : elements) {
            if (element.second->IsActive()) {
                element.second->Render();
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        wd->ClearValue.color.float32[0] = clearColor.x * clearColor.w;
        wd->ClearValue.color.float32[1] = clearColor.y * clearColor.w;
        wd->ClearValue.color.float32[2] = clearColor.z * clearColor.w;
        wd->ClearValue.color.float32[3] = clearColor.w;
        vulkan.FrameRender(wd, draw_data);
        vulkan.FramePresent(wd);
    }
}
