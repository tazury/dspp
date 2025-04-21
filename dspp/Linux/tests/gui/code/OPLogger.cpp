#include "OPLogger.h"
#include <cstdlib>     // For system()
#include <algorithm>

void showLinuxMessageBox(const std::string& title, const std::string& message) {
    std::string escapedMsg = message;
    std::replace(escapedMsg.begin(), escapedMsg.end(), '"', '\''); // Escape quotes

    // Try zenity first (GNOME/Wayland compatible)
    std::string cmd = "zenity --error --title=\"" + title + "\" --text=\"" + escapedMsg + "\"";
    if (system(cmd.c_str()) != 0) {
        // Try kdialog (KDE)
        cmd = "kdialog --error \"" + escapedMsg + "\" --title \"" + title + "\"";
        if (system(cmd.c_str()) != 0) {
            // Fallback: terminal output
            std::cerr << "[CRITICAL ERROR]: " << message << std::endl;
        }
    }
}

void setConsoleColor(int textColor, int bgColor = ConsoleColor::Black) {
    std::cout << "\033[" << textColor << ";" << (bgColor + 10) << "m";
}

// Function to reset console color
void resetConsoleColor() {
    std::cout << "\033[0m";
}

    bool OPLogger::CustomLog(std::string output, int color) {
        if (color == -1) color = 0;
        setConsoleColor(color);
        std::cout << output << std::endl;
        resetConsoleColor();
        return true;
    }

    bool OPLogger::WError(std::wstring output, int critical) {
        std::string converted(output.begin(), output.end());
        return Error(converted, critical);
}

    bool OPLogger::DediError(WDediErrorD data) {
        std::wstring full = data.Header + L"\n\n" + data.Description;
        if (data.enAdvancedInfo)
            full += L"\n\n" + data.AdvancedInfoTitle + L"\n" + data.AdvancedInfoC;

        std::string finalStr(full.begin(), full.end());

        showLinuxMessageBox("Dedicated Error - " + std::string(data.Title.begin(), data.Title.end()), finalStr);
        exit(1);
        return true;
    }

    bool OPLogger::Error(std::string output, int critical) {
        setConsoleColor(31);
        if (critical == 0) {
            std::cout << "[ERROR]: " << output << std::endl;
        } else {
            std::cout << "[CRITICAL ERROR]: " << output << std::endl;
            resetConsoleColor();

#ifdef _WIN32
            std::wstring wideOutput(output.begin(), output.end());
        MessageBox(NULL, wideOutput.c_str(), L"Critical Error!", MB_ICONERROR | MB_OK);
#else
            showLinuxMessageBox("Critical Error!", output);
#endif
            exit(1);
        }
        resetConsoleColor();
        return true;
    }

    void OPLogger::clear() {
        system("cls");
    }

    OPLogger OPLogger::get() {
        static OPLogger OPL;
        return OPL;
    }
