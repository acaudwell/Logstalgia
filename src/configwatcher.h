#ifndef CONFIG_WATCHER_H
#define CONFIG_WATCHER_H

#include "FileWatcher/FileWatcher.h"

#include <string>

class ConfigWatcher : public FW::FileWatchListener {
protected:
    FW::FileWatcher* watcher;
    std::string config_file;
    bool changed;
public:
    ConfigWatcher();
    virtual ~ConfigWatcher();

    void setConfig(const std::string& config_file);

    bool start();

    void stop();

    void update();

    bool changeDetected();

    void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
        FW::Action action);
};

#endif
