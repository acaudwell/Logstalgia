/*
    Copyright (C) 2013 Andrew Caudwell (acaudwell@gmail.com)

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

#ifndef LOGSTALGIA_SETTINGS_H
#define LOGSTALGIA_SETTINGS_H

#define LOGSTALGIA_VERSION "1.0.6"

#include "core/settings.h"

#define PADDLE_NONE   0
#define PADDLE_SINGLE 1
#define PADDLE_PID    2
#define PADDLE_VHOST  3

class LogstalgiaSettings : public SDLAppSettings {
protected:
    void commandLineOption(const std::string& name, const std::string& value);
public:
    int log_level;
    bool ffp;

    std::string path;
    std::vector<std::string> groups;

    std::string load_config;
    std::string save_config;

    time_t start_time;
    time_t stop_time;

    float splash;

    float simulation_speed;
    float pitch_speed;
    float update_rate;

    int   paddle_mode;
    float paddle_position;

    float start_position;
    float stop_position;

    bool sync;

    bool hide_response_code;
    bool hide_url_prefix;
    bool hide_paddle;
    bool hide_paddle_tokens;

    bool no_bounce;

    bool disable_auto_skip;
    bool disable_progress;
    bool disable_glow;

    bool mask_hostnames;

    vec3 background_colour;

    float glow_intensity;
    float glow_multiplier;
    float glow_duration;

    int font_size;

    LogstalgiaSettings();

    void setLogstalgiaDefaults();

    void importLogstalgiaSettings(ConfFile& conf, ConfSection* settings = 0);

    void help(bool extended_help=false);
};

extern LogstalgiaSettings settings;

#endif
