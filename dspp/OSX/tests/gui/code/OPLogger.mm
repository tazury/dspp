#include "OPLogger.h"
#include <cstdlib>     // For system()
#include <algorithm>
#import <Cocoa/Cocoa.h>

void showMessageBox(const std::string& title, const std::string& message) {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithUTF8String:title.c_str()];
    alert.informativeText = [NSString stringWithUTF8String:message.c_str()];
    [alert addButtonWithTitle:@"OK"];
    [alert setAlertStyle:NSAlertStyleCritical];

    // Run modal (blocking)
    [alert runModal];
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

        showMessageBox("Dedicated Error - " + std::string(data.Title.begin(), data.Title.end()), finalStr);
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

            showMessageBox("Critical Error!", output);

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
