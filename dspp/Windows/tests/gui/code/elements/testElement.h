#pragma once
#include "../UIElement.h"
#include "../window.h"
#include <include/DualSenseMgr.h>
#include <include/Logger.h>

class WindowElement : public UIElement {
public:
    WindowElement(const std::string& name, Window* window)
        : UIElement(name), win(window) {
    }

    void OnDestroy() override {
        mgr->~DualSenseMgr();
    }

    void OnInit() override {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        Resource data = win->vulkan.resources.LoadData(101);

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
        large_font = io.Fonts->AddFontFromMemoryTTF(data.data, data.size, 74.0f, &font_cfg);
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


        if(controllers.empty())  ImGui::Text(text);  // Render the text

       
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

            style.WindowPadding = ImVec2(9.0f, 4.0f);
            // Output Section - left side of the screen
            ImVec2 output_size = ImVec2(screen_size.x * 0.3f, screen_size.y - player_list_size.y); // 30% width and remaining height after player list
            ImVec2 output_pos = ImVec2(0, player_list_size.y); // Start below the player list
            ImGui::SetNextWindowPos(output_pos);
            ImGui::SetNextWindowSize(output_size);
            ImGui::Begin("Output Section", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Output Section - This is how you control the output of your controller");
            auto& output = controllers[playerIDC-1]->output;

            uint8_t min = 0x00;
            uint8_t max = 0xff;

            // LED Settings Section
            if (ImGui::CollapsingHeader("LED Settings")) {
                ImGui::Checkbox("Mute LED", &output.ledSettings.muteLED);



                // Option and brightness settings
                ImGui::SliderScalar("Option", ImGuiDataType_U8, &output.ledSettings.option, &min, &max);
                ImGui::SliderScalar("Brightness", ImGuiDataType_U8, &output.ledSettings.brightness, &min, &max);

                // Pulse Option setting
                ImGui::SliderScalar("Pulse Option", ImGuiDataType_U8, &output.ledSettings.pulseOption, &min, &max);

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
                ImGui::SliderScalar("Mode", ImGuiDataType_U8, &output.leftTrigger.mode, &min, &max);
                ImGui::SliderScalar("Force 1", ImGuiDataType_U8, &output.leftTrigger.force1, &min, &max);
                ImGui::SliderScalar("Force 2", ImGuiDataType_U8, &output.leftTrigger.force2, &min, &max);
                ImGui::SliderScalar("Force 3", ImGuiDataType_U8, &output.leftTrigger.force3, &min, &max);
                ImGui::SliderScalar("Force 4", ImGuiDataType_U8, &output.leftTrigger.force4, &min, &max);
                ImGui::SliderScalar("Force 5", ImGuiDataType_U8, &output.leftTrigger.force5, &min, &max);
                ImGui::SliderScalar("Force 6", ImGuiDataType_U8, &output.leftTrigger.force6, &min, &max);
                ImGui::SliderScalar("Force 7", ImGuiDataType_U8, &output.leftTrigger.force7, &min, &max);
            }

            if (ImGui::CollapsingHeader("Right Trigger Settings")) {
                ImGui::SliderScalar("Mode", ImGuiDataType_U8, &output.rightTrigger.mode, &min, &max);
                ImGui::SliderScalar("Force 1", ImGuiDataType_U8, &output.rightTrigger.force1, &min, &max);
                ImGui::SliderScalar("Force 2", ImGuiDataType_U8, &output.rightTrigger.force2, &min, &max);
                ImGui::SliderScalar("Force 3", ImGuiDataType_U8, &output.rightTrigger.force3, &min, &max);
                ImGui::SliderScalar("Force 4", ImGuiDataType_U8, &output.rightTrigger.force4, &min, &max);
                ImGui::SliderScalar("Force 5", ImGuiDataType_U8, &output.rightTrigger.force5, &min, &max);
                ImGui::SliderScalar("Force 6", ImGuiDataType_U8, &output.rightTrigger.force6, &min, &max);
                ImGui::SliderScalar("Force 7", ImGuiDataType_U8, &output.rightTrigger.force7, &min, &max);
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
            ImVec2 input_size = ImVec2(screen_size.x * 0.3f, screen_size.y - player_list_size.y); // 30% width and remaining height after player list
            ImVec2 input_pos = ImVec2(screen_size.x - input_size.x, player_list_size.y); // Start below the player list, aligned to the right
            ImGui::SetNextWindowPos(input_pos);
            ImGui::SetNextWindowSize(input_size);
            ImGui::Begin("Input Section", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Input Section");
            ImGui::End();

            // Controller Section - center of the screen
            ImVec2 controller_size = ImVec2(screen_size.x * 0.4f, screen_size.y - player_list_size.y); // 40% width, full height minus player list height
            ImVec2 controller_pos = ImVec2((screen_size.x - controller_size.x) * 0.5f, player_list_size.y); // Center horizontally, start below the player list
            ImGui::SetNextWindowPos(controller_pos);
            ImGui::SetNextWindowSize(controller_size);
            ImGui::Begin("Controller Section", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text("Controller Section (Live Controller will appear here)");
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
};