#pragma once

#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <iostream>
#include "imgui/imgui.h"
#include <typeinfo>
#include "OPLogger.h"


class UIElement {
public:
    explicit UIElement(const std::string& name)
        : name(name), initialized(false), isActive(true) {
    }
    bool operator==(const UIElement& other) const {
        // Implement the comparison logic (e.g., compare members)
        return this->name == other.name;
    }
    // Custom deinit
    virtual ~UIElement() = default;
    virtual void SetWindowSize(ImVec2 newWindowSize) {}
    // Lifecycle hooks
    virtual void OnInit() {
#if !isRetail
        DebugLog("Init");
#endif
    }
    virtual void OnUpdate() {}
    virtual void OnDestroy() {}

    virtual void RenderChildren() {
        if (!isActive) return;

        for (auto& child : children) {
            child.second->Render();
        }
    }

    virtual void Render() {
        if (!isActive) return;


        if (!initialized) {
            OnInit();
            initialized = true;
        }

        OnUpdate();

        for (auto& child : children) {
            child.second->Render();
        }
    }

    // Debug output function
    void DebugLog(std::string message, int color = -1) const {
        const char* classname = typeid(*this).name();
        const char* nameC = name.c_str();
        const char* messageC = message.c_str();
#if !isRetail
        OPLogger::get().CustomLog("[Debug : {} : {}] {}", color, classname, nameC, messageC);
#endif
    }

    void DebugError(std::string message, int type = 0) const {
        const char* classname = typeid(*this).name();
        const char* nameC = name.c_str();
        const char* messageC = message.c_str();
#if !isRetail
        OPLogger::get().Error("[Error : {} : {}] {}", type, classname, nameC, messageC);
#endif
    }

    // Add a child element
    void AddChild(std::string id, std::shared_ptr<UIElement> child) {
        children[id] = child;
    }

    // Remove a child element
    void RemoveChild(std::shared_ptr<UIElement> child) {
        auto it = std::find_if(children.begin(), children.end(),
            [&child](const std::pair<std::string, std::shared_ptr<UIElement>>& pair) {
                return pair.second == child;  // Compare the shared_ptrs
            });

        if (it != children.end()) {
            children.erase(it);
        }
    }

    void RemoveChild(std::string id) {
        auto idElement = children.find(id);
        if (idElement != children.end()) {
            children.erase(idElement);
        }
    }


    // Access children
    const std::map<std::string, std::shared_ptr<UIElement>>& GetChildren() const {
        return children;
    }

    // Accessors
    const std::string& GetName() const { return name; }
    bool IsActive() const { return isActive; }

    // Activation / Deactivation
    void SetActive(bool active) { isActive = active; }

    // Activation toggle
    void ToggleActive() { isActive = !isActive; }
    std::string name;
    bool initialized;
    bool isActive;
    std::map<std::string, std::shared_ptr<UIElement>> children;
};


