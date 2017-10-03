/*
    Copyright (C) 2017 Andrew Caudwell (acaudwell@gmail.com)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIG_WATCHER_H
#define CONFIG_WATCHER_H

#include <string>
#include <time.h>

class ConfigWatcher {
protected:
    std::string config_file;
    bool changed;
    time_t last_mtime;
    float elapsed;
public:
    ConfigWatcher();
    virtual ~ConfigWatcher();

    void setConfig(const std::string& config_file);

    void logic(float dt);

    bool changeDetected();
};

#endif
