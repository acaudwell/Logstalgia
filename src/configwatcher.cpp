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

#include "configwatcher.h"

#include "core/logger.h"
#include "core/sdlapp.h"

#include <sys/types.h>
#include <sys/stat.h>

ConfigWatcher::ConfigWatcher()
    : changed(false), last_mtime(0), elapsed(0.0f) {
}

ConfigWatcher::~ConfigWatcher() {
}

void ConfigWatcher::setConfig(const std::string &config_file) {
    this->config_file = config_file;
    elapsed = 0.0f;
    changed = false;

    struct stat st;
    int rc = stat(config_file.c_str(), &st);

    if(rc == 0) {
        last_mtime = st.st_mtime;
    } else {
        last_mtime = time(0);
    }
}

void ConfigWatcher::logic(float dt) {

    elapsed += dt;

    if(elapsed >= 1.0f) {
        elapsed = 0.0f;

        if(!changed) {
            struct stat st;
            int rc = stat(config_file.c_str(), &st);

            if(rc == 0) {
                time_t mtime = st.st_mtime;

                if(mtime > this->last_mtime) {
                    this->last_mtime = mtime;
                    changed = true;
                }
            }
        }
   }
}

bool ConfigWatcher::changeDetected() {
    bool has_changed = changed;
    changed = false;
    return has_changed;
}
