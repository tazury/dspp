// This is not an common file [Win32]
#include "Logger.h"
#include <mutex>

std::unordered_map<int, DSLogger::Listener> DSLogger::listeners_;
std::atomic<int> DSLogger::nextId_{ 0 };
std::mutex DSLogger::mutex_;

int DSLogger::addListener(Listener callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = nextId_++;
    listeners_[id] = callback;
    return id;
}

void DSLogger::removeListener(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.erase(id);
}

void DSLogger::log(const std::string& message, int type,
    const std::string& file, int line) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (listeners_.empty()) return;

    LogEntry entry{ type, message, makeRelativePath(file), line };
    for (const auto& [id, callback] : listeners_) {
        callback(entry);
    }
}

std::string DSLogger::baseDir_ = "";

void DSLogger::setBaseDirectory(const std::string& baseDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    baseDir_ = baseDir;

    // Normalize path separators and ensure trailing slash
    std::replace(baseDir_.begin(), baseDir_.end(), '\\', '/');
    if (!baseDir_.empty() && baseDir_.back() != '/') {
        baseDir_ += '/';
    }
}

std::string DSLogger::makeRelativePath(const std::string& absPath) {
    std::string path = absPath;
    std::replace(path.begin(), path.end(), '\\', '/');

    if (!baseDir_.empty()) {
        size_t basePos = path.find(baseDir_);
        if (basePos != std::string::npos) {
            return path.substr(basePos + baseDir_.length());
        }
    }
    return path;
}