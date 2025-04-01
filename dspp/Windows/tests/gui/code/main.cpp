#include "window.h"
#include <include/Logger.h>


int main() {
    DSLogger::addListener([](const LogEntry& entry) {
        const char* typeStr = "Unknown";
        switch (entry.type) {
        case DSLogTypes::Normal: typeStr = "Normal"; break;
        case DSLogTypes::Warning: typeStr = "Warning"; break;
        case DSLogTypes::Error: typeStr = "Error"; break;
        }
        std::cout << "[" << typeStr << "] "
            << entry.file << ":" << entry.line << " - "
            << entry.message << "\n";
        });

	Window window(1920, 720, "test");
	window.Run();
}