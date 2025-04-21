#pragma once
#include "../UIElement.h"
#include "../window.h"
#include <DualSenseMgr.h>
#include <Logger.h>
#include <imgui/imgui_internal.h>
#include <iostream>
#include <fstream>

class WindowElement : public UIElement {
public:
    WindowElement(const std::string& name, Window* window)
        : UIElement(name), win(window) {
    }
    void RegisterController(int playerID, DualSense::DualSense* ctrl) {
        if (!activeControllers[playerID]) {
            activeControllers[playerID] = true;
            ctrl->input.addListener([this, playerID](DualSense::Input::InputState state, double timestamp) {
                controllerStates[playerID] = state; // Store the latest input state
                });
            ctrl->input.start();
        }
    }
    void OnDestroy() override {
        mgr->~DualSenseMgr();
    }

    
    void RenderAnalogSticks(float lx, float ly, float rx, float ry) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        float radius = 40.0f;

        // Left Stick
        ImVec2 leftCenter = { cursor.x + radius, cursor.y + radius };
        ImVec2 leftStickPos = { leftCenter.x + lx * radius, leftCenter.y + ly * radius };

        // Right Stick (placed 120px to the right for spacing)
        ImVec2 rightCenter = { leftCenter.x + 120.0f, leftCenter.y };
        ImVec2 rightStickPos = { rightCenter.x + rx * radius, rightCenter.y + ry * radius };

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Draw Left Stick Circle
        drawList->AddCircle(leftCenter, radius, IM_COL32(255, 255, 255, 255), 32);
        drawList->AddText(ImVec2(leftStickPos.x - 4, leftStickPos.y - 6), IM_COL32(255, 0, 0, 255), "L");

        // Draw Right Stick Circle
        drawList->AddCircle(rightCenter, radius, IM_COL32(255, 255, 255, 255), 32);
        drawList->AddText(ImVec2(rightStickPos.x - 4, rightStickPos.y - 6), IM_COL32(0, 255, 0, 255), "R");

        // Reserve space
        ImGui::Dummy(ImVec2(200, radius * 2));
    }

    void OnInit() override {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        //Resource data = win->vulkan.resources.LoadData(101);
        // We are loading the font from an file.

        std::ifstream file("./font2", std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: font2" << std::endl;
            OPLogger::get().Error("Unable to open fonts", 1);
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        std::vector<uint8_t> buffer(size);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), size);

        DualSenseMgr::DiscoverSettings set;
        set.maxPlayers = 4;

        mgr->discoverInBackground(set, [&](DualSenseMgr::CtrlIncomingInfo info) {

            if (info.reconnection) {
                // disconnectedCtrls[disconnectedCtrls.size()-1].
            }

            return true;

            });

        DebugLog("WindowElement Debug Init");
        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;  // Prevent ImGui from freeing memory
        large_font = io.Fonts->AddFontFromMemoryTTF(buffer.data(), buffer.size(), 74.0f, &font_cfg);
        ImGui_ImplVulkan_CreateFontsTexture();

        
        initialized = true;
    }

    void Render() override {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        bool alt = io.KeyAlt;
        bool shift = io.KeyShift;
        bool d_released = ImGui::IsKeyReleased(ImGuiKey_D);

        if (alt && shift && d_released)
        {
            // Alt + Shift + D was released
            show_demo_window ? show_demo_window = false : show_demo_window = true;
        }

      

        std::map<int, DualSense::DualSense*> controllers = mgr->getControllers();

        const char* text;
      
        if (controllers.empty()) {
            text = "Connect to your DualSense controller";
        } else text = "Hello player";

        ImVec2 window_size = ImGui::GetIO().DisplaySize;  // Get the size of the window
        ImVec2 text_size = ImGui::CalcTextSize(text);  // Get the size of the text

        ImVec2 window_padding = ImGui::GetStyle().WindowPadding;
        window_padding.x = 0;

        // Calculate the position to center the text
        ImVec2 text_pos = ImVec2(
            (window_size.x - text_size.x) * 0.5f,  // Center horizontally
            (window_size.y - text_size.y) * 0.5f   // Center vertically
        );
            


        ImGui::SetCursorPos(text_pos);  // Set the cursor to the calculated position

        ImGui::PushFont(large_font);


        if(controllers.empty())  ImGui::Text("%s", text);  // Render the text

       
        ImGui::PopFont();


        if (!controllers.empty()) {
            ImVec2 screen_size = ImGui::GetIO().DisplaySize;
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowPadding = ImVec2(5.0f, 4.0f);

            ImVec2 player_list_size = ImVec2(screen_size.x, 50.0f); // Small window at the top
            ImVec2 player_list_pos = ImVec2(0, 0); // Top-left corner
            ImGui::SetNextWindowPos(player_list_pos);
            ImGui::SetNextWindowSize(player_list_size);
            ImGui::Begin("Player List", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoBackground);

            // Loop through and create buttons for each player
            for (auto [id, controller] : controllers) {
                if (ImGui::Button(("Player " + std::to_string(id+1)).c_str())) {
                    OPLogger::get().CustomLog("Does nothing");
                }
            }
            ImGui::End();

            // Start with default percentages if not resized yet
            static float controllerWidth = screen_size.x * 0.4f;

            // Optional: Clamp it so it never becomes too small or large
            float minWidth = screen_size.x * 0.2f;
            float maxWidth = screen_size.x * 0.6f;
            controllerWidth = ImClamp(controllerWidth, minWidth, maxWidth);

            // Calculate the other two widths
            float sideWidth = (screen_size.x - controllerWidth) * 0.5f;

            style.WindowPadding = ImVec2(9.0f, 4.0f);
            // Output Section - left side of the screen
            ImVec2 output_size = ImVec2(sideWidth, screen_size.y - player_list_size.y);
            ImVec2 output_pos = ImVec2(0, player_list_size.y);
            ImGui::SetNextWindowPos(output_pos);
            ImGui::SetNextWindowSize(output_size);
            ImGui::Begin("Output Section", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::TextWrapped("Output Section - This is how you control the output of your controller");
            auto& output = controllers[playerIDC-1]->output;

            uint8_t min = 0x00;
            uint8_t max = 0xff;
            uint8_t max2 = 0x02;

            ImGui::SliderScalar("Validity Flags", ImGuiDataType_U8, &output.validityFlag, &min, &max);

            // LED Settings Section
            if (ImGui::CollapsingHeader("LED Settings")) {
                ImGui::Checkbox("Mute LED", &output.ledSettings.muteLED);
           

                // Player number setting
                ImGui::SliderScalar("Player Number", ImGuiDataType_U8, &output.ledSettings.playerNumber, &min, &max);

                // Color settings (RGB sliders)
                float color[3] = {
    output.ledSettings.colors.red / 255.0f,
    output.ledSettings.colors.green / 255.0f,
    output.ledSettings.colors.blue / 255.0f
                };

                // Display the color picker and modify the color
                if (ImGui::ColorEdit3("LED Color", color)) {
                    // If the color changes, convert back to uint8_t
                    output.ledSettings.colors.red = (uint8_t)(color[0] * 255.0f);
                    output.ledSettings.colors.green = (uint8_t)(color[1] * 255.0f);
                    output.ledSettings.colors.blue = (uint8_t)(color[2] * 255.0f);
                }
            }

            // Trigger Settings Section
            if (ImGui::CollapsingHeader("Left Trigger Settings")) {
                ImGui::SliderScalar("L Mode", ImGuiDataType_U8, &output.leftTrigger.mode, &min, &max);
                ImGui::SliderScalar("L Force 1", ImGuiDataType_U8, &output.leftTrigger.force1, &min, &max);
                ImGui::SliderScalar("L Force 2", ImGuiDataType_U8, &output.leftTrigger.force2, &min, &max);
                ImGui::SliderScalar("L Force 3", ImGuiDataType_U8, &output.leftTrigger.force3, &min, &max);
                ImGui::SliderScalar("L Force 4", ImGuiDataType_U8, &output.leftTrigger.force4, &min, &max);
                ImGui::SliderScalar("L Force 5", ImGuiDataType_U8, &output.leftTrigger.force5, &min, &max);
                ImGui::SliderScalar("L Force 6", ImGuiDataType_U8, &output.leftTrigger.force6, &min, &max);
                ImGui::SliderScalar("L Force 7", ImGuiDataType_U8, &output.leftTrigger.force7, &min, &max);
            }

            if (ImGui::CollapsingHeader("Right Trigger Settings")) {
                ImGui::SliderScalar("R Mode", ImGuiDataType_U8, &output.rightTrigger.mode, &min, &max);
                ImGui::SliderScalar("R Force 1", ImGuiDataType_U8, &output.rightTrigger.force1, &min, &max);
                ImGui::SliderScalar("R Force 2", ImGuiDataType_U8, &output.rightTrigger.force2, &min, &max);
                ImGui::SliderScalar("R Force 3", ImGuiDataType_U8, &output.rightTrigger.force3, &min, &max);
                ImGui::SliderScalar("R Force 4", ImGuiDataType_U8, &output.rightTrigger.force4, &min, &max);
                ImGui::SliderScalar("R Force 5", ImGuiDataType_U8, &output.rightTrigger.force5, &min, &max);
                ImGui::SliderScalar("R Force 6", ImGuiDataType_U8, &output.rightTrigger.force6, &min, &max);
                ImGui::SliderScalar("R Force 7", ImGuiDataType_U8, &output.rightTrigger.force7, &min, &max);
            }

            // Motor Settings Section
            if (ImGui::CollapsingHeader("Motor Settings")) {
                ImGui::SliderScalar("Overall Motors", ImGuiDataType_U8, &output.overallMotors, &min, &max);
                ImGui::SliderScalar("Left Motor", ImGuiDataType_U8, &output.leftMoter, &min, &max);
                ImGui::SliderScalar("Right Motor", ImGuiDataType_U8, &output.rightMoter, &min, &max);
            }


            output.sendOutput();
            ImGui::End();

            // Input Section - right side of the screen
            ImVec2 input_size = ImVec2(sideWidth, screen_size.y - player_list_size.y);
            ImVec2 input_pos = ImVec2(screen_size.x - sideWidth, player_list_size.y);
            ImGui::SetNextWindowPos(input_pos);
            ImGui::SetNextWindowSize(input_size);
            ImGui::Begin("Input Section", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            if (!activeControllers[playerIDC]) {
                RegisterController(playerIDC, controllers[playerIDC - 1]);
            }

            auto& state = controllerStates[playerIDC];
            
            ImGui::TextWrapped("Player %d", playerIDC);

            // Button States
            ImGui::TextWrapped("Buttons:");
            for (int i = 0; i < static_cast<int>(DualSense::Input::Button::_Count); i++) {
                if (state.buttons[i]) {
                    ImGui::TextWrapped("%d: Pressed", i);
                }
            }

            // Axis (Stick) Data
            RenderAnalogSticks(state.axes[static_cast<size_t>(DualSense::Input::Axis::LeftStickX)], state.axes[static_cast<size_t>(DualSense::Input::Axis::LeftStickY)], state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickX)], state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickY)]);
        //    RenderAnalogStick("Right Stick", state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickX)], state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickY)]);
            ImGui::TextWrapped("Left Stick: X=%.2f Y=%.2f", state.axes[static_cast<size_t>(DualSense::Input::Axis::LeftStickX)], state.axes[static_cast<size_t>(DualSense::Input::Axis::LeftStickY)]);
            ImGui::TextWrapped("Right Stick: X=%.2f Y=%.2f", state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickX)], state.axes[static_cast<size_t>(DualSense::Input::Axis::RightStickY)]);

            // Triggers
            ImGui::TextWrapped("L2: %.2f (Effect: %d)", state.axes[4], state.triggers[0].inEffect);
            ImGui::TextWrapped("R2: %.2f (Effect: %d)", state.axes[5], state.triggers[1].inEffect);

            // Touchpad
            if (state.touchPoints[0].active)
                ImGui::TextWrapped("Touch 1: ID=%d X=%ld Y=%ld", state.touchPoints[0].id, state.touchPoints[0].x, state.touchPoints[0].y);
            if (state.touchPoints[1].active)
                ImGui::TextWrapped("Touch 2: ID=%d X=%ld Y=%ld", state.touchPoints[1].id, state.touchPoints[1].x, state.touchPoints[1].y);

            // Motion Sensors
            ImGui::TextWrapped("Accelerometer: X=%.2f Y=%.2f Z=%.2f", state.motion.accelerometer[0], state.motion.accelerometer[1], state.motion.accelerometer[2]);
            ImGui::TextWrapped("Gyroscope: X=%.2f Y=%.2f Z=%.2f", state.motion.gyroscope[0], state.motion.gyroscope[1], state.motion.gyroscope[2]);

            if (state.buttons[static_cast<size_t>(DualSense::Input::Button::Mute)]) {
                state.microphoneMuted = !state.microphoneMuted;
            }

            // Microphone
            ImGui::TextWrapped("Microphone Muted: %s", state.microphoneMuted ? "Yes" : "No");

            ImGui::TextWrapped("Input Section");
            ImGui::End();

            // Controller Section - center of the screen
            ImVec2 controller_size = ImVec2(controllerWidth, screen_size.y - player_list_size.y);
            ImVec2 controller_pos = ImVec2(sideWidth, player_list_size.y);
            ImGui::SetNextWindowPos(controller_pos);
            ImGui::SetNextWindowSize(controller_size, ImGuiCond_Always);
            ImGui::Begin("Controller Section", NULL,  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Controller Section (Live Controller will appear here)");
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 controllerPos = ImGui::GetCursorScreenPos();

            ImVec2 newSize = ImGui::GetWindowSize();
            if (newSize.x != controllerWidth) {
                controllerWidth = newSize.x;
            }

            drawList->AddRectFilled(controllerPos, ImVec2(controllerPos.x + 300, controllerPos.y + 180), IM_COL32(50, 50, 50, 255));

            // Render Analog Sticks
            RenderAnalogSticks(state.axes[0], state.axes[1], state.axes[2], state.axes[3]);

            // Highlight Buttons
            for (int i = 0; i < static_cast<int>(DualSense::Input::Button::_Count); i++) {
                if (state.buttons[i]) {
                    ImGui::Text("Button %d Pressed", i);
                }
            }



            // Touchpad Interaction
            if (state.touchPoints[0].active)
                drawList->AddCircleFilled(ImVec2(controllerPos.x + state.touchPoints[0].x * 0.3f, controllerPos.y + state.touchPoints[0].y * 0.3f), 5.0f, IM_COL32(0, 0, 255, 255));
            if (state.touchPoints[1].active)
                drawList->AddCircleFilled(ImVec2(controllerPos.x + state.touchPoints[1].x * 0.3f, controllerPos.y + state.touchPoints[1].y * 0.3f), 5.0f, IM_COL32(0, 255, 255, 255));


            ImGui::End();
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);


        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        //{
        //    static float f = 0.0f;
        //    static int counter = 0;

        //    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        //    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        //    ImGui::Checkbox("Another Window", &show_another_window);

        //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //    ImGui::ColorEdit4("clear color", (float*)&win->uiEngine->clearColor); // Edit 3 floats representing a color

        //    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //        counter++;
        //    ImGui::SameLine();
        //    ImGui::Text("counter = %d", counter);

        //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //    ImGui::End();
        //}

        //// 3. Show another simple window.
        //if (show_another_window)
        //{
        //    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //    ImGui::Text("Hello from another window!");
        //    if (ImGui::Button("Close Me"))
        //        show_another_window = false;
        //    ImGui::End();
        //}
    }

private:
    bool show_demo_window = true;
    bool show_another_window = false;
    Window* win;
    ImFont* large_font;
    int playerIDC = 1;
    DualSenseMgr* mgr = new DualSenseMgr();
    std::map<int, DualSense::DualSense*> disconnectedCtrls;
    std::map<int, bool> activeControllers;
    std::map<int, DualSense::Input::InputState> controllerStates;
};