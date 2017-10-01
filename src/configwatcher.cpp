#include "configwatcher.h"

#include "core/logger.h"
#include "core/sdlapp.h"

ConfigWatcher::ConfigWatcher()
    : watcher(0), changed(false) {
}

ConfigWatcher::~ConfigWatcher() {
    if(watcher != 0) delete watcher;
}

void ConfigWatcher::setConfig(const std::string &config_file) {
    this->config_file = config_file;
}

bool ConfigWatcher::start() {
    if(watcher != 0) stop();

    std::string config_dir;

    size_t last_slash = config_file.rfind('/');
#ifdef _WIN32
    size_t last_back_slash = config_file.rfind('\\');
    if(   last_back_slash != std::string::npos
       && (last_slash == std::string::npos || last_back_slash > last_slash)) {
        last_slash = last_back_slash;
    }
#endif

    if(last_slash != std::string::npos) {
        config_dir  = config_file.substr(0,last_slash+1);
        config_file = config_file.substr(last_slash+1, config_file.size()-last_slash);
    }

    if(config_dir.empty()) {
        config_dir = ".";
    }

    debugLog("config_file = %s", config_file.c_str());
    debugLog("config_dir = %s", config_dir.c_str());

    watcher = new FW::FileWatcher();

    try {
        watcher->addWatch(config_dir, this);
    } catch(FW::FileNotFoundException& e) {
        debugLog("failed to create file watcher for directory %s", config_dir.c_str());
        stop();
        return false;
    }

    return true;
}

void ConfigWatcher::stop() {
    delete watcher;
    watcher = 0;
    changed = false;
}

void ConfigWatcher::update() {
    if(watcher != 0) watcher->update();
}

bool ConfigWatcher::changeDetected() {
    bool has_changed = changed;
    changed = false;
    return has_changed;
}

void ConfigWatcher::handleFileAction(FW::WatchID watchid, const FW::String &dir, const FW::String &filename, FW::Action action)
{
    if(action == FW::Actions::Modified || action == FW::Actions::Add) {
        if(filename == config_file) {
            changed = true;
        }
    }
}
