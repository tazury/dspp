#pragma once
/*
* THIS IS DESIGNED FOR CLION with the Linux! If that isn't your platform, please check if there are any others!

* Title: DS++ Logger
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "Just an logger for you to add an listener to. If you want, you can use this as your logger. After all, you send the logs
            The information is given to you or sent by you."

* Compatibility: Designed in C++ 20 | C Standard 17

* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/

#include <string>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <algorithm>

#define DS_LOG(msg, type) DSLogger::log(msg, type, __FILE__, __LINE__)
#define DS_LOG_NORMAL(msg) DS_LOG(msg, DSLogTypes::Normal)
#define DS_LOG_WARNING(msg) DS_LOG(msg, DSLogTypes::Warning)
#define DS_LOG_ERROR(msg) DS_LOG(msg, DSLogTypes::Error)
#define DS_BASEDIR(dir) DSLogger::setBaseDirectory(dir)

namespace DSLogTypes {
    constexpr int Normal = 0;
    constexpr int Warning = 1;
    constexpr int Error = 2;
}

struct LogEntry {
    int type;           // Log type (use DSLogTypes constants)
    std::string message; // Log message
    std::string file;    // Source file where log originated
    int line;           // Line number in source file
};

class DSLogger {
public:
    using Listener = std::function<void(const LogEntry&)>;

    // Add a listener and return its ID
    static int addListener(Listener callback);

    // Remove a listener by ID
    static void removeListener(int id);

    // Core logging method
    static void log(const std::string& message, int type,
        const std::string& file, int line);

    static void setBaseDirectory(const std::string& baseDir);

private:
    static std::unordered_map<int, Listener> listeners_;
    static std::atomic<int> nextId_;
    static std::mutex mutex_;
    static std::string baseDir_;  // Add this member variable
    static std::string makeRelativePath(const std::string& absPath);
};
