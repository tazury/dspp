#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_vulkan.h>
#include <iostream>
#include "VulkanHelpers.h"
#include "UIEngine.h"

#define SDL_ENABLE_OLD_NAMES 1

class Window
{
public:
	Window(int width, int height, const char* title);
	~Window();
	void Run();
public:
    UIEngine* uiEngine = nullptr;
    VulkanHelpers vulkan;
private:
    void Cleanup();
    SDL_Window* window;
    bool running;
};

