#include "window.h"
#include "elements/testElement.h"

Window::Window(int width, int height, const char* title){

    running = true;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to init SDL3");
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    window = SDL_CreateWindow(title, width, height, window_flags);
    if (!window) {
        throw std::runtime_error("Failed to create SDL3 window");
    }

    std::vector<const char*> extensions;
    {
        uint32_t sdl_extensions_count = 0;
        const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        for (uint32_t n = 0; n < sdl_extensions_count; n++)
            extensions.push_back(sdl_extensions[n]);
    }

    vulkan.SetupVulkan(extensions);
    
    VkSurfaceKHR surface;
    VkResult err{};
    if (SDL_Vulkan_CreateSurface(window, vulkan.instance, vulkan.allocator, &surface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        return;
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &vulkan.mainWindowD;
    vulkan.SetupVulkanWindow(wd, surface, w, h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    float scale = SDL_GetWindowDisplayScale(window) * 1.5f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui::StyleColorsDark();



    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = vulkan.instance;
    init_info.PhysicalDevice = vulkan.physDevice;
    init_info.Device = vulkan.device;
    init_info.QueueFamily = vulkan.queueFamily;
    init_info.Queue = vulkan.queue;
    init_info.PipelineCache = vulkan.pipelineCache;
    init_info.DescriptorPool = vulkan.descriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = vulkan.minImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = vulkan.allocator;
    init_info.CheckVkResultFn = VulkanHelpers::checkVkResult;
    ImGui_ImplVulkan_Init(&init_info);

    vulkan.UpdateUIScaling(scale > 1.0f ? scale : 1.0f);
    uiEngine = new UIEngine(window, vulkan);

    auto winElementUI = std::make_shared<WindowElement>(
        "winElement", this
    );

    uiEngine->AddElement(winElementUI);

}

Window::~Window()
{
    Cleanup();
}

void Window::Run()
{
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui_ImplVulkanH_Window* wd = &vulkan.mainWindowD;
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            //if (event.type == SDL_EVENT_WINDOW_RESIZED)
            //{
            //    int width, height;
            //    SDL_GetWindowSize(window, &width, &height);
            //    ImGui_ImplVulkan_SetMinImageCount(vulkan.minImageCount);
            //    ImGui_ImplVulkanH_CreateOrResizeWindow(vulkan.instance, vulkan.physDevice, vulkan.device, &vulkan.mainWindowD, vulkan.minImageCount, vulkan.allocator, width, height, vulkan.minImageCount);
            //    vulkan.mainWindowD.FrameIndex = 0;
            //    vulkan.swapchainRebuild = false;
            //}
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

      

        uiEngine->Render();

       
    }
}



void Window::Cleanup()
{
    uiEngine->~UIEngine();
}
