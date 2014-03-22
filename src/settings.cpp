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

#include "settings.h"

#include "core/logger.h"
#include "core/sdlapp.h"
#include "core/seeklog.h"

LogstalgiaSettings settings;

//display help message
void LogstalgiaSettings::help(bool extended_help) {

#ifdef _WIN32
    //resize window to fit help message
    SDLApp::resizeConsole(710);
    SDLApp::showConsole(true);
#endif

    printf("Logstalgia v%s\n", LOGSTALGIA_VERSION);

    printf("Usage: logstalgia [OPTIONS] FILE\n\n");
    printf("Options:\n");
    printf("  -WIDTHxHEIGHT              Set window size\n");
    printf("  -f                         Fullscreen\n\n");

    printf("  -b --background FFFFFF     Background colour in hex\n\n");

    printf("  -x --full-hostnames        Show full request ip/hostname\n");
    printf("  -s --simulation-speed      Simulation speed (default: 1)\n");
    printf("  -p --pitch-speed           Speed balls travel across screen (default: 0.15)\n");
    printf("  -u --update-rate           Page summary update rate (default: 5)\n\n");

    printf("  -g name,(HOST|URI|CODE)=regex,percent[,colour]\n");
    printf("                             Group together requests where the HOST, URI\n");
    printf("                             or response CODE matches a regular expression\n\n");

    printf("  --paddle-mode MODE         Paddle mode (single, pid, vhost)\n");
    printf("  --paddle-position POSITION Paddle position as a fraction of the view width\n\n");

    printf("  --sync                     Read from STDIN, ignoring entries before now\n\n");

    printf("  --from, --to 'YYYY-MM-DD hh:mm:ss'  Show entries from a specific time period\n\n");

    printf("  --start-position POSITION  Begin at some position in the log (0.0 - 1.0)\n");
    printf("  --stop-position  POSITION  Stop at some position\n\n");

    printf("  --no-bounce                No bouncing\n\n");

    printf("  --hide-response-code       Hide response code\n");
    printf("  --hide-paddle              Hide paddle\n");
    printf("  --hide-paddle-tokens       Hide paddle tokens shown in multi-paddle modes\n");
    printf("  --hide-url-prefix          Hide URL protocol and hostname prefix\n\n");

    printf("  --disable-auto-skip        Disable skipping of empty time periods\n");
    printf("  --disable-progress         Disable the progress bar\n");
    printf("  --disable-glow             Disable the glow effect\n\n");

    printf("  --font-size SIZE           Font size\n\n");

    printf("  --glow-duration            Duration of the glow (default: 0.15)\n");
    printf("  --glow-multiplier          Adjust the amount of glow (default: 1.25)\n");
    printf("  --glow-intensity           Intensity of the glow (default: 0.5)\n\n");

    printf("  --load-config CONF_FILE    Load a config file\n");
    printf("  --save-config CONF_FILE    Save a config file with the current options\n\n");

    printf("  -o, --output-ppm-stream FILE   Write frames as PPM to a file ('-' for STDOUT)\n");
    printf("  -r, --output-framerate  FPS    Framerate of output (25,30,60)\n\n");

    printf("FILE should be a log file or '-' to read STDIN.\n\n");

    if(extended_help) {
    }

#ifdef _WIN32
    if(!SDLApp::existing_console) {
        printf("Press Enter\n");
        getchar();
    }
#endif

    exit(0);
}

LogstalgiaSettings::LogstalgiaSettings() {
    log_level = LOG_LEVEL_OFF;
    splash    = -1.0f;

    setLogstalgiaDefaults();

    default_section_name = "logstalgia";

    //translate args
    arg_aliases["g"] = "group";
    arg_aliases["p"] = "start-position";
    arg_aliases["b"] = "background";
    arg_aliases["x"] = "full-hostnames";
    arg_aliases["u"] = "update-rate";
    arg_aliases["s"] = "simulation-speed";
    arg_aliases["speed"] = "simulation-speed";
    arg_aliases["p"] = "pitch-speed";
    arg_aliases["c"] = "splash";
    arg_aliases["H"] = "extended-help";
    arg_aliases["h"] = "help";
    arg_aliases["?"] = "help";

    //command line only options
    conf_sections["help"]            = "command-line";
    conf_sections["extended-help"]   = "command-line";
    conf_sections["load-config"]     = "command-line";
    conf_sections["save-config"]     = "command-line";
    conf_sections["log-level"]       = "command-line";
    conf_sections["splash"]          = "command-line";

    // arg types

    arg_types["font-size"] = "int";

    arg_types["help"]          = "bool";
    arg_types["extended-help"] = "bool";
    arg_types["splash"]        = "bool";

    arg_types["sync"]            = "bool";
    arg_types["full-hostnames"]  = "bool";
    arg_types["no-bounce"]       = "bool";
    arg_types["ffp"]             = "bool";

    arg_types["hide-paddle"]        = "bool";
    arg_types["hide-paddle-tokens"] = "bool";
    arg_types["hide-response-code"] = "bool";

    arg_types["disable-auto-skip"] = "bool";
    arg_types["disable-progress"]  = "bool";
    arg_types["disable-glow"]      = "bool";

    arg_types["glow-intensity"]   = "float";
    arg_types["glow-multiplier"]  = "float";
    arg_types["glow-duration"]    = "float";
    arg_types["paddle-position"]  = "float";

    arg_types["pitch-speed"]      = "float";
    arg_types["simulation-speed"] = "float";
    arg_types["update-rate"]      = "float";

    arg_types["group"] = "multi-value";

    arg_types["to"]                 = "string";
    arg_types["from"]               = "string";
    arg_types["log-level"]          = "string";
    arg_types["load-config"]        = "string";
    arg_types["save-config"]        = "string";
    arg_types["path"]               = "string";
    arg_types["background"]         = "string";
    arg_types["start-position"]     = "string";
    arg_types["stop-position"]      = "string";
    arg_types["paddle-mode"]        = "string";
}

void LogstalgiaSettings::setLogstalgiaDefaults() {

    path = "";

    sync = false;

    start_time = stop_time = 0;

    start_position = 0.0f;
    stop_position  = 1.0f;

    paddle_mode     = PADDLE_SINGLE;
    paddle_position = 0.67f;

    pitch_speed       = 0.15f;
    simulation_speed  = 1.0f;
    update_rate       = 5.0f;

    glow_intensity  = 0.5f;
    glow_multiplier = 1.25f;
    glow_duration   = 0.15f;

    disable_auto_skip  = false;
    disable_progress   = false;
    disable_glow       = false;

    hide_response_code = false;
    hide_paddle        = false;
    hide_url_prefix    = false;
    hide_paddle_tokens = false;

    no_bounce          = false;

    mask_hostnames = true;

    ffp = false;

    background_colour = vec3(0.0f, 0.0f, 0.0f);

    font_size = 14;

    groups.clear();
}

void LogstalgiaSettings::commandLineOption(const std::string& name, const std::string& value) {

    if(name == "help") {
        help();
    }

    if(name == "extended-help") {
        help(true);
    }

    if(name == "splash") {
        splash = 10.0f;
        return;
    }

    if(name == "load-config" && value.size() > 0) {
        load_config = value;
        return;
    }

    if(name == "save-config" && value.size() > 0) {
        save_config = value;
        return;
    }

    if(name == "log-level") {
        if(value == "warn") {
            log_level = LOG_LEVEL_WARN;
        } else if(value == "debug") {
            log_level = LOG_LEVEL_DEBUG;
        } else if(value == "info") {
            log_level = LOG_LEVEL_INFO;
        } else if(value == "error") {
            log_level = LOG_LEVEL_ERROR;
        } else if(value == "pedantic") {
            log_level = LOG_LEVEL_PEDANTIC;
        }
        return;
    }

    std::string invalid_error = std::string("invalid ") + name + std::string(" value");
    throw ConfFileException(invalid_error, "", 0);
}

void LogstalgiaSettings::importLogstalgiaSettings(ConfFile& conffile, ConfSection* settings) {

    setLogstalgiaDefaults();

    if(settings == 0) settings = conffile.getSection(default_section_name);

    if(settings == 0) {
        settings = conffile.addSection("logstalgia");
    }

    ConfEntry* entry = 0;

    if((entry = settings->getEntry("glow-intensity")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify glow-intensity (float)");

        glow_intensity = entry->getFloat();

        if(glow_intensity <= 0.0f) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("glow-multiplier")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify glow-multiplier (float)");

        glow_multiplier = entry->getFloat();

        if(glow_multiplier <= 0.0f) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("glow-duration")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify glow-duration (float)");

        glow_duration = entry->getFloat();

        if(glow_duration <= 0.0f || glow_duration > 1.0f) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("font-size")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify font size");

        font_size = entry->getInt();

        if(font_size<1 || font_size>100) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("background")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify background colour (FFFFFF)");

        int r,g,b;

        std::string colstring = entry->getString();

        if(entry->isVec3()) {
            background_colour = entry->getVec3();
        } else if(colstring.size()==6 && sscanf(colstring.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
            background_colour = vec3(r,g,b);
            background_colour /= 255.0f;
        } else {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("from")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify from (YYYY-MM-DD hh:mm:ss)");

        if(!parseDateTime(entry->getString(), start_time)) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("to")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify to (YYYY-MM-DD hh:mm:ss)");

        if(!parseDateTime(entry->getString(), stop_time)) {
            conffile.invalidValueException(entry);
        }
    }

    if((entry = settings->getEntry("start-position")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify start-position (float,random)");

        start_position = entry->getFloat();

        if(start_position<=0.0 || start_position>=1.0) {
            conffile.entryException(entry, "start-position outside of range 0.0 - 1.0 (non-inclusive)");
        }
    }

    if((entry = settings->getEntry("stop-position")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify stop-position (float)");

        stop_position = entry->getFloat();

        if(stop_position<=0.0 || stop_position>1.0) {
            conffile.entryException(entry, "stop-position outside of range 0.0 - 1.0 (inclusive)");
        }
    }

    if((entry = settings->getEntry("group")) != 0) {

        ConfEntryList* group_entries = settings->getEntries("group");

        for(ConfEntry* entry : *group_entries) {
            if(!entry->hasValue()) conffile.entryException(entry, "specify group definition");
            groups.push_back(entry->getString());
        }
    }

    if((entry = settings->getEntry("paddle-mode")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify paddle-mode (vhost,pid)");

        std::string paddle_mode_string = entry->getString();

        if(paddle_mode_string == "single") {
            paddle_mode = PADDLE_SINGLE;

        } else if(paddle_mode_string == "pid") {
            paddle_mode = PADDLE_PID;

        } else if(paddle_mode_string == "vhost") {
            paddle_mode = PADDLE_VHOST;

        } else {
            conffile.entryException(entry, "invalid paddle-mode");
        }
    }

    if((entry = settings->getEntry("paddle-position")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify paddle-position (0.25 - 0.75)");

        paddle_position = entry->getFloat();

        if(paddle_position < 0.25f || paddle_position > 0.75f) {
            conffile.entryException(entry, "paddle-position outside of range 0.25 - 0.75");
        }
    }

    if((entry = settings->getEntry("pitch-speed")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify pitch speed (0.1 to 10.0)");

        pitch_speed = entry->getFloat();

        if(pitch_speed < 0.1f || pitch_speed > 10.0f) {
            conffile.entryException(entry, "pitch speed should be between 0.1 and 10.0");
        }
    }

    if((entry = settings->getEntry("simulation-speed")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify simulation speed (0.1 to 30)");

        simulation_speed = entry->getFloat();

        if(simulation_speed < 0.1f || simulation_speed > 30.0f) {
            conffile.entryException(entry, "simulation speed should be between 0.1 and 30");
        }
    }

    if((entry = settings->getEntry("update-rate")) != 0) {

        if(!entry->hasValue()) conffile.entryException(entry, "specify update rate (1 to 60)");

        update_rate = entry->getFloat();

        if(update_rate < 1.0f || update_rate > 60.0f) {
            conffile.entryException(entry, "update rate should be between 1 and 60");
        }
    }

    if(settings->getBool("sync")) {
        sync = true;
    }

    if(settings->getBool("hide-paddle")) {
        paddle_mode = PADDLE_NONE;
    }

    if(settings->getBool("hide-paddle-tokens")) {
        hide_paddle_tokens = true;
    }

    if(settings->getBool("hide-response-code")) {
        hide_response_code = true;
    }

    if(settings->getBool("no-bounce")) {
        no_bounce = true;
    }

    if(settings->getBool("disable-auto-skip")) {
        disable_auto_skip = true;
    }

    if(settings->getBool("disable-progress")) {
        disable_progress = true;
    }

    if(settings->getBool("disable-glow")) {
        disable_glow = true;
    }

    if(settings->getBool("full-hostnames")) {
        mask_hostnames = false;
    }

    if(settings->getBool("hide-url-prefix")) {
        hide_url_prefix = true;
    }

    if(settings->getBool("ffp")) {
        ffp = true;
    }

    //validate path
    if(settings->hasValue("path")) {
        path = settings->getString("path");
    }

    if (path.empty() && !isatty(fileno(stdin))) {
        path = "-";
    }

    if(path == "-") {
        /*
        if(log_format.size() == 0) {
            throw ConfFileException("log-format required when reading from STDIN", "", 0);
        }*/

#ifdef _WIN32
        DWORD available_bytes;
        HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

        while(PeekNamedPipe(stdin_handle, 0, 0, 0,
            &available_bytes, 0) && available_bytes==0 && !std::cin.fail()) {
            SDL_Delay(100);
        }
#else
        while(std::cin.peek() == EOF && !std::cin.fail()) SDL_Delay(100);
#endif

        std::cin.clear();
    }
}
