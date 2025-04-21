#pragma once

#include <vector>
#include <memory>
#include "UIElement.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_metal.h"
#include "imgui/imgui_impl_osx.h"
#include <string>


#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class UIEngine
{
public:
    UIEngine();
	UIEngine(id <MTLDevice> device, id <MTLCommandQueue> commandQueue);
	~UIEngine();

	void Render(MTKView* view);
    void AddElement(std::shared_ptr<UIElement> element, std::string id = "") {
        if (id.empty()) {
            id = element->name;
        }
        elements[id] = element;
        element->OnInit();
    }

    std::shared_ptr<UIElement> getElementAsBase(std::string id) {
        return elements.find(id)->second;
    }

    template <typename T>
    std::shared_ptr<T> getElementAs(const std::string& id) {
        auto it = elements.find(id);
        if (it != elements.end()) {
            // Attempt to cast to the desired type
            std::shared_ptr<T> derived = std::dynamic_pointer_cast<T>(it->second);
            if (derived) {
                return derived;
            }
            else {
                OPLogger::get().Error("Element with ID '" + id + "' is not of the requested type.", 0);
                return nullptr;
            }
        }
        else {
            OPLogger::get().Error("Element with ID '" + id + "' not found.", 0);
            return nullptr;
        }
    }

    // Remove an element from the engine
    void RemoveElement(std::shared_ptr<UIElement> element) {
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            if (it->second == element) {
                elements.erase(it);
                break;
            }
        }
    }

    void RemoveElement(std::string id) {
        auto idElement = elements.find(id);
        if (idElement != elements.end()) {
            elements.erase(idElement);
        }
    }
public:
    ImVec4 clearColor = ImVec4(0, 0, 0, 1.00f);
    bool isReal = false;
    ImVec2 currentSize;
private:
    id <MTLDevice> device;
    id <MTLCommandQueue> commandQueue;
    std::map<std::string, std::shared_ptr<UIElement>> elements;
};

