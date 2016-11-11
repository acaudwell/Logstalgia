/*
    Copyright (C) 2008 Andrew Caudwell (acaudwell@gmail.com)

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

#include "logstalgia.h"
#include "settings.h"
#include "ncsa.h"
#include "custom.h"
#include "configwatcher.h"

#include "core/png_writer.h"
#include "core/timezone.h"

//Logstalgia

//turn performance profiling
//#define LS_PERFORMANCE_PROFILE

std::string profile_name;
Uint32 profile_start_msec;

void profile_start(std::string profile) {
#ifdef LS_PERFORMANCE_PROFILE
    profile_start_msec = SDL_GetTicks();
    profile_name = profile;
#endif
}

void profile_stop() {
#ifdef LS_PERFORMANCE_PROFILE
    debugLog("%s took %d ms\n", profile_name.c_str(), SDL_GetTicks() - profile_start_msec);
#endif
}

void logstalgia_info(std::string msg) {
    SDLAppInfo(msg);
}

void logstalgia_quit(std::string error) {
    SDLAppQuit(error);
}

Logstalgia::Logstalgia(const std::string& logfile) : SDLApp() {
    info       = false;
    paused     = false;
    retarget   = false;
    next       = false;

    initialized = false;

    this->logfile = logfile;

    spawn_delay=0;

    highscore = 0;

    message_timer = 0.0f;

    ipSummarizer  = 0;

    mintime       = settings.sync ? time(0) : settings.start_time;
    seeklog       = 0;
    streamlog     = 0;

    if(logfile.empty()) {
        throw SDLAppException("no file supplied");
    }

    if(logfile == "-") {
        streamlog = new StreamLog();

    } else {
        try {
            seeklog = new SeekLog(logfile);

        } catch(SeekLogException& exception) {
            throw SDLAppException("unable to read log file");
        }
    }

    total_space = 0;
    remaining_space = 0;

    total_entries=0;

    balltex = 0;
    glowtex = 0;

    mousehide_timeout = 0.0f;

    runtime = 0.0;
    frameExporter = 0;
    framecount = 0;
    frameskip = 0;
    fixed_tick_rate = 0.0;

    accesslog = 0;

    font_alpha = 1.0;

    take_screenshot = false;

    //every 60 minutes seconds blank text for 60 seconds

    screen_blank_interval = 3600.0;
    screen_blank_period   = 60.0;
    screen_blank_elapsed  = 0.0;

    adjusting_size = false;
    default_cursor = SDL_GetDefaultCursor();
    resize_cursor  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);

    detect_changes = settings.detect_changes;
    config_watcher = 0;

    init_tz();
}

Logstalgia::~Logstalgia() {
    if(accesslog!=0) delete accesslog;

    for(auto& it: paddles) {
        delete it.second;
    }
    paddles.clear();

    if(seeklog!=0) delete seeklog;
    if(streamlog!=0) delete streamlog;
    if(config_watcher!=0) delete config_watcher;

    for(auto& it : summarizer_types) {
        if(it.second != 0) delete it.second;
    }

    if(ipSummarizer!=0) delete ipSummarizer;

    for(Summarizer* s : summarizers) {
        delete s;
    }

    summarizers.clear();
    summarizer_types.clear();

}

void Logstalgia::togglePause() {
    paused = !paused;

    if(!paused) {
        if(settings.sync) {
            mintime = time(0);
            elapsed_time = mintime - starttime;
        }
    }
}

void Logstalgia::keyPress(SDL_KeyboardEvent *e) {
    if (e->type == SDL_KEYDOWN) {

        if (e->keysym.sym == SDLK_ESCAPE) {
            appFinished=true;
        }

        if(e->keysym.sym == SDLK_q) {
            info = !info;
        }

        if(e->keysym.sym == SDLK_c) {
            settings.splash = 10.0f;
        }

        if(e->keysym.sym == SDLK_n) {
            next = true;
        }

        if (e->keysym.sym == SDLK_p) {
            if(GLEW_VERSION_2_0) {
                settings.ffp = !settings.ffp;
            }
        }

        if (e->keysym.sym == SDLK_s && e->keysym.mod & KMOD_CTRL) {
            saveConfig();
        }

        if (e->keysym.sym == SDLK_F5) {
            reloadConfig();
        }

        if (e->keysym.sym == SDLK_F6) {
            loadConfig();
        }

        if(e->keysym.sym == SDLK_SPACE) {
            togglePause();
        }

        if (e->keysym.sym == SDLK_F11) {
            toggleWindowFrame();
        }

        if (e->keysym.sym == SDLK_F12) {
            take_screenshot = true;
        }

        if(!settings.sync) {

            if (e->keysym.sym == SDLK_EQUALS || e->keysym.sym == SDLK_KP_PLUS) {
                if(settings.simulation_speed >= 1.0f) {
                    settings.simulation_speed = std::min(30.0f, glm::floor(settings.simulation_speed) + 1.0f);
                } else {
                    settings.simulation_speed = std::min(1.0f, settings.simulation_speed * 2.0f);
                }
                retarget=true;
            }

            if (e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_MINUS) {
                if(settings.simulation_speed > 1.0f) {
                    settings.simulation_speed = std::max(1.0f, glm::floor(settings.simulation_speed) - 1.0f);
                } else {
                    settings.simulation_speed = std::max(0.1f, settings.simulation_speed * 0.5f);
                }
                retarget=true;
            }
        }

        if(e->keysym.sym == SDLK_PERIOD) {
            settings.pitch_speed = glm::clamp(settings.pitch_speed+0.1f, 0.0f, 10.0f);
            retarget=true;
        }

        if(e->keysym.sym == SDLK_COMMA) {
            settings.pitch_speed = glm::clamp(settings.pitch_speed-0.1f, 0.1f, 10.0f);
            retarget=true;
        }

        if(e->keysym.sym == SDLK_HOME) {
            changeIPSummarizerDepth(-1);
        }

        if(e->keysym.sym == SDLK_END) {
            changeIPSummarizerDepth(+1);
        }

        if(e->keysym.sym == SDLK_INSERT) {
            changeGroupSummarizerMode();
        }

        if(e->keysym.sym == SDLK_PAGEUP) {
            changeGroupSummarizerDepth(-1);
        }

        if(e->keysym.sym == SDLK_PAGEDOWN) {
            changeGroupSummarizerDepth(+1);
        }

        if(e->keysym.sym == SDLK_RETURN && e->keysym.mod & KMOD_ALT) {
            toggleFullscreen();
        }
    }
}


void Logstalgia::initPaddles() {

    paddle_x = display.width * settings.paddle_position;
    paddle_colour = (settings.paddle_mode > PADDLE_SINGLE) ?
        vec4(0.0f, 0.0f, 0.0f, 0.0f) : vec4(0.5, 0.5, 0.5, 1.0);

    if(!paddles.empty()) {
        for(auto& it: paddles) {
            delete it.second;
        }
        paddles.clear();
    }

    if(settings.paddle_mode <= PADDLE_SINGLE) {
        vec2 paddle_pos = vec2(paddle_x - 20, rand() % display.height);
        Paddle* paddle = new Paddle(paddle_pos, paddle_colour, "", fontSmall);
        paddles[""] = paddle;
    }
}

void Logstalgia::initRequestBalls() {

    for(RequestBall* r: balls) {
        removeBall(r);
    }

    balls.clear();
}

void Logstalgia::reset() {

    end_reached = false;

    highscore = 0;

    initPaddles();
    initRequestBalls();

    ipSummarizer->recalc_display();

    for(Summarizer* s : summarizers) {
        s->recalc_display();
    }

    for(LogEntry* l : queued_entries) {
        delete l;
    }
    queued_entries.clear();

    // reset settings
    elapsed_time  = 0;
    starttime     = 0;
    lasttime      = 0;
}

void Logstalgia::saveConfig() {

    // update display settings

    settings.display_width  = display.width;
    settings.display_height = display.height;

    settings.fullscreen = display.isFullscreen();
    settings.frameless  = display.isFrameless();

    if(!settings.fullscreen) {
        SDL_GetWindowPosition(display.sdl_window, &settings.window_x, &settings.window_y);
    } else {
        settings.window_x = -1;
        settings.window_y = -1;
    }

#ifdef _WIN32
    if(settings.load_config.empty()) {
        //get original directory
        char cwd_buff[1024];

        if(getcwd(cwd_buff, 1024) != cwd_buff) {
            SDLAppQuit("error getting current working directory");
        }

        OPENFILENAME ofn;

        char filepath[_MAX_PATH];
        filepath[0] = '\0';

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = 0;
        ofn.lpstrFile = filepath;
        ofn.nMaxFile = sizeof(filepath);
        ofn.lpstrFilter = "Config File (*.cfg)\0*.cfg\0*.*\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = 0;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = 0;
        ofn.Flags = OFN_PATHMUSTEXIST;

        GetSaveFileName(&ofn);

        //change back to original directory
        if(chdir(cwd_buff) != 0) {
            SDLAppQuit("error changing directory");
        }

        settings.load_config = std::string(filepath);
    }
#endif

    if(settings.load_config.empty()) return;

    ConfFile conf;
    settings.exportDisplaySettings(conf);
    settings.exportLogstalgiaSettings(conf);

    std::string config_file = settings.load_config;

    try {
        conf.save(config_file);

        // TODO: stop message disappearing too quickly due to large dt value
        // while waiting for the user to choose the save location

        setMessage("Wrote config %s", config_file.c_str());
    } catch(ConfFileException&) {
        setMessage("Failed to save config to %s", config_file.c_str());
    }
}

void Logstalgia::loadConfig() {
#ifdef _WIN32
    //get original directory
    char cwd_buff[1024];

    if(getcwd(cwd_buff, 1024) != cwd_buff) {
        SDLAppQuit("error getting current working directory");
    }

    OPENFILENAME ofn;

    char filepath[_MAX_PATH];
    filepath[0] = '\0';

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = 0;
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = sizeof(filepath);
    ofn.lpstrFilter = "Config File (*.cfg)\0*.cfg\0*.*\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = 0;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = 0;
    ofn.Flags = OFN_PATHMUSTEXIST;

    GetOpenFileName(&ofn);

    //change back to original directory
    if(chdir(cwd_buff) != 0) {
        SDLAppQuit("error changing directory");
    }

    loadConfig(std::string(filepath));
#endif
}

void Logstalgia::loadConfig(const std::string& config_file) {

    try {
        ConfFile conf;
        conf.load(config_file);

        // validate config
        LogstalgiaSettings new_settings;
        new_settings.importLogstalgiaSettings(conf);

        if(settings.path != new_settings.path) {

            if(settings.path == "-" || new_settings.path == "-") {
                throw ConfFileException("cannot change streaming mode at run time", config_file, 0);
            }

            SeekLog* new_seeklog = 0;

            try {
                new_seeklog = new SeekLog(new_settings.path);
            }
            catch(SeekLogException& e) {
                throw ConfFileException("unable to read log file", config_file, 0);
            }

            if(seeklog != 0) delete seeklog;
            seeklog = new_seeklog;

            debugLog("path changed from %s to %s", settings.path.c_str(), new_settings.path.c_str());
        }

        // figure out if we should call init()

        bool reinitialize = false;

        if(   settings.path != new_settings.path
           || settings.font_size != new_settings.font_size
           || settings.groups.size() != new_settings.groups.size()) {

            reinitialize = true;

        } else {
            for(int i = 0; i < settings.groups.size();i++) {
                if(settings.groups[i] != new_settings.groups[i]) {
                    reinitialize = true;
                    break;
                }
            }
        }

        // if no exceptions were thrown, import settings
        settings.importLogstalgiaSettings(conf);

        settings.load_config = config_file;

        paused = false;

        if(reinitialize) {
            init();
        }

    } catch(ConfFileException& e) {
       setMessage(e.what());
    }
}

void Logstalgia::reloadConfig() {
    if(settings.load_config.empty()) return;
    loadConfig(settings.load_config);
}

void Logstalgia::changeGroupSummarizerMode() {
    for(Summarizer* s : summarizers) {
        Summarizer::SortMode sort_mode = s->getSortMode();
        if(sort_mode == Summarizer::DELIMITER_COUNT) {
            sort_mode = Summarizer::WORD_COUNT;
        } else {
            sort_mode = Summarizer::DELIMITER_COUNT;
        }
        debugLog("%s sort mode changed to %s mode", s->getTitle().c_str(),
                 (sort_mode == Summarizer::DELIMITER_COUNT) ? "delimiter" : "word");
        s->setSortMode(sort_mode);
        s->summarize();
        s->recalc_display();
    }
}

void Logstalgia::changeSummarizerDepth(Summarizer* summarizer, int delta) {
    int depth = summarizer->getAbbreviationDepth();
    int new_depth = depth + delta;

    if(new_depth >= -1) {
        debugLog("%s depth changed to %d", summarizer->getTitle().c_str(), new_depth);
        summarizer->setAbbreviationDepth(new_depth);
        summarizer->summarize();
        summarizer->recalc_display();
    }
}

void Logstalgia::changeIPSummarizerDepth(int delta) {
    changeSummarizerDepth(ipSummarizer, delta);
}

void Logstalgia::changeGroupSummarizerDepth(int delta) {
    for(Summarizer* s : summarizers) {
        changeSummarizerDepth(s, delta);
    }
}

void Logstalgia::screenshot() {

    //get next free recording name
    char pngname[256];
    struct stat finfo;
    int png_no = 1;

    while(png_no < 10000) {
        snprintf(pngname, 256, "logstalgia-%04d.png", png_no);
        if(stat(pngname, &finfo) != 0) break;
        png_no++;
    }

    //write png
    std::string filename(pngname);

    PNGWriter png;
    png.screenshot(filename);

    setMessage("Wrote screenshot %s", pngname);
}

void Logstalgia::setMessage(const char* str, ...) {

    char msgbuff[1024];

    va_list vl;

    va_start(vl, str);
        vsnprintf(msgbuff, 1024, str, vl);
    va_end(vl);

    message = std::string(msgbuff);
    message_timer = 5.0;
}

bool Logstalgia::hasProgressBar() {
    return seeklog != 0 && !settings.disable_progress;
}

void Logstalgia::seekTo(float percent) {

    if(!seeklog) return;

    //disable pause if enabled before seeking
    if(paused) paused = false;

    reset();

    seeklog->seekTo(percent);

    readLog();
}

void Logstalgia::mouseClick(SDL_MouseButtonEvent *e) {

    if(e->type != SDL_MOUSEBUTTONDOWN) {
        adjusting_size = false;
        return;
    }

    if(e->button == SDL_BUTTON_LEFT) {

        if(hasProgressBar()) {
            float position;
            if(slider.click(mousepos, &position)) {
                seekTo(position);
            }
        }

//        if(!ipSummarizer->setPrefixFilterAtPos(mousepos)) {
//        }

        for(Summarizer* s : summarizers) {
            if(s->setPrefixFilterAtPos(mousepos)) break;
        }

        if(mouseOverSummarizerWidthAdjuster(mousepos)) {
            adjusting_size = true;
        }
    }

    if(e->button == SDL_BUTTON_RIGHT) {
        for(Summarizer* s: summarizers) {
            if(s->mouseOver(mousepos) && !s->getPrefixFilter().empty()) {
                s->setPrefixFilter("");
                break;
            }
        }
    }
}

//peek at the date under the mouse pointer on the slider
std::string Logstalgia::dateAtPosition(float percent) {

    std::string date;

    if(seeklog == 0 || accesslog == 0) return date;

    //get line at position

    std::string linestr;

    if(percent<1.0 && seeklog->getNextLineAt(linestr, percent)) {

        LogEntry le;

        set_utc_tz();

        bool parsed = accesslog->parseLine(linestr, le);

        unset_utc_tz();

        if(parsed) {
            //display date
            char datestr[256];

            time_t timestamp = le.timestamp;

            struct tm* timeinfo = localtime ( &timestamp );
            strftime(datestr, 256, "%H:%M:%S %B %d, %Y", timeinfo);
            date = std::string(datestr);
        }
     }

    return date;
}

bool Logstalgia::mouseOverSummarizerWidthAdjuster(const vec2& pos) {
    return (paddle_x >= (pos.x - 3) && paddle_x <= (pos.x + 3));
}

void Logstalgia::changePaddleX(float x) {

    paddle_x = x;

    for(Summarizer* s : summarizers) {
        s->setPosX(paddle_x);
    }

    float paddle_offset = paddle_x - 20;

    for(auto it : paddles) {
        it.second->setX(paddle_offset);
    }

    for(RequestBall* ball: balls) {
        ball->changeDestX(paddle_offset);
    }
}

void Logstalgia::mouseMove(SDL_MouseMotionEvent *e) {
    mousepos = vec2(e->x, e->y);
    SDL_ShowCursor(true);
    mousehide_timeout = 5.0f;

    float pos;

    if(hasProgressBar() && slider.mouseOver(mousepos, &pos)) {
        std::string date = dateAtPosition(pos);
        slider.setCaption(date);
    }

    if(adjusting_size) {

        // dont allow adjusting size while an animation is happening

        bool can_adjust = true;

        for(Summarizer* s : summarizers) {
            if(s->isAnimating()) {
                can_adjust = false;
                break;
            }
        }

        if(can_adjust) {
            changePaddleX(mousepos.x);
        }
    }

    if(mouseOverSummarizerWidthAdjuster(mousepos)) {
        SDL_SetCursor(resize_cursor);
    } else {
        SDL_SetCursor(default_cursor);
    }
}

Regex ls_url_hostname_regex("^http://[^/]+(.+)$");

std::string Logstalgia::filterURLHostname(const std::string& hostname) {

    std::vector<std::string> matches;

    if(ls_url_hostname_regex.match(hostname, &matches)) {
        return matches[0];
    }

    return hostname;
}

Summarizer* Logstalgia::getGroupSummarizer(LogEntry* le) {

    auto host_match_summarizers = summarizer_types["HOST"];
    auto code_match_summarizers = summarizer_types["CODE"];
    auto uri_match_summarizers  = summarizer_types["URI"];

    if(host_match_summarizers != 0) {
        for(Summarizer* s : *host_match_summarizers) {
            if(s->supportedString(le->hostname)) {
                return s;
            }
        }
    }

    if(code_match_summarizers != 0) {
        for(Summarizer* s : *code_match_summarizers) {
            if(s->supportedString(le->response_code)) {
                return s;
            }
        }
    }

    if(uri_match_summarizers != 0) {
        for(Summarizer* s : *uri_match_summarizers) {
            if(s->supportedString(le->path)) {
                return s;
            }
        }
    }

    return 0;
}

void Logstalgia::addStrings(LogEntry* le) {

    Summarizer* groupSummarizer = getGroupSummarizer(le);

    if(!groupSummarizer) return;

    std::string hostname = le->hostname;
    std::string pageurl  = le->path;

    if(settings.hide_url_prefix) pageurl = filterURLHostname(pageurl);

    groupSummarizer->addString(pageurl);
    ipSummarizer->addString(hostname);
}

void Logstalgia::addBall(LogEntry* le, float start_offset) {

    //find appropriate summarizer for url
    Summarizer* groupSummarizer = getGroupSummarizer(le);

    if(!groupSummarizer) return;

    Paddle* entry_paddle = 0;

    if(settings.paddle_mode > PADDLE_SINGLE) {

        std::string paddle_token = (settings.paddle_mode == PADDLE_VHOST) ? le->vhost : le->pid;

        entry_paddle = paddles[paddle_token];

        if(entry_paddle == 0) {
            vec2 paddle_pos = vec2(paddle_x - 20, rand() % display.height);
            Paddle* paddle = new Paddle(paddle_pos, paddle_colour, paddle_token, fontSmall);
            entry_paddle = paddles[paddle_token] = paddle;
        }

    } else {
        entry_paddle = paddles[""];
    }

    std::string hostname = le->hostname;
    std::string pageurl  = le->path;

    if(settings.hide_url_prefix) pageurl = filterURLHostname(pageurl);

    float dest_y = groupSummarizer->getMiddlePosY(pageurl);
    float pos_y  = ipSummarizer->getMiddlePosY(hostname);

    float start_x = -(entry_paddle->getX() * settings.pitch_speed * start_offset);

    //debugLog("start_offset %.2f : start_x = %.2f (paddle_x %.2f, pitch_speed %.2f)", start_offset, start_x, entry_paddle->getX(), settings.pitch_speed);

    vec2 ball_start = vec2(start_x, pos_y);
    vec2 ball_dest  = vec2(entry_paddle->getX(), dest_y);

    const std::string& match = ipSummarizer->getBestMatchStr(hostname);

    vec3 colour = groupSummarizer->hasColour() ? groupSummarizer->getColour() : colourHash(match);

    RequestBall* ball = new RequestBall(le, colour, ball_start, ball_dest);

    balls.push_back(ball);
}

BaseLog* Logstalgia::getLog() {
    if(seeklog !=0) return seeklog;

    return streamlog;
}

void Logstalgia::readLog(int buffer_rows) {

    profile_start("readLog");

    set_utc_tz();

    int entries_read = 0;

    std::string linestr;
    BaseLog* baselog = getLog();

    time_t read_timestamp = 0;

    while( baselog->getNextLine(linestr) ) {

        //trim whitespace
        if(linestr.size()>0) {
            size_t string_end =
                linestr.find_last_not_of(" \t\f\v\n\r");

            if(string_end == std::string::npos) {
                linestr = "";
            } else if(string_end != linestr.size()-1) {
                linestr = linestr.substr(0,string_end+1);
            }
        }

        LogEntry le;

        bool parsed_entry;

        //determine format
        if(accesslog==0) {

            //is this a recognized NCSA access log?
            NCSALog* ncsalog = new NCSALog();
            if((parsed_entry = ncsalog->parseLine(linestr, le))) {
                accesslog = ncsalog;
            } else {
                delete ncsalog;
            }

            if(accesslog==0) {
                //is this a custom log?
                CustomAccessLog* customlog = new CustomAccessLog();
                if((parsed_entry = customlog->parseLine(linestr, le))) {
                    accesslog = customlog;
                } else {
                    delete customlog;
                }
            }

        } else {

            if(!(parsed_entry = accesslog->parseLine(linestr, le))) {
                debugLog("error: could not read line %s\n", linestr.c_str());
            }
        }

        if(parsed_entry) {

            if((!mintime || mintime <= le.timestamp) && (!settings.stop_time || settings.stop_time > le.timestamp)) {

                queued_entries.push_back(new LogEntry(le));

                total_entries++;
                entries_read++;

                //read at least the buffered row count if specified
                //otherwise read all entries with the same time
                if(buffer_rows) {
                    if(entries_read > buffer_rows) break;
                } else {
                    if(read_timestamp && read_timestamp < le.timestamp) break;
                }

                read_timestamp = le.timestamp;
            }
        }
    }

    profile_stop();

    unset_utc_tz();

    if(queued_entries.empty() && seeklog != 0) {

        if(total_entries==0) {
            if(mintime != 0) {
                logstalgia_quit("could not parse any entries in the specified time period");
            } else {
                logstalgia_quit("could not parse any entries");
            }
        }

        //no more entries
        end_reached = true;

        return;
    }

    if(seeklog != 0) {
        float percent = seeklog->getPercent();

        if(percent > settings.stop_position) {
            end_reached = true;
            return;
        }

        if(hasProgressBar()) slider.setPercent(percent);
    }

    //set start time if currently 0
    if(starttime==0 && !queued_entries.empty()) {
        starttime = queued_entries.front()->timestamp;
        currtime  = 0;
    }
}

void Logstalgia::init() {

    fontLarge  = fontmanager.grab("FreeSerif.ttf", 42);
    fontMedium = fontmanager.grab("FreeMonoBold.ttf", 16);
    fontBall   = fontmanager.grab("FreeMonoBold.ttf", 16);
    fontSmall  = fontmanager.grab("FreeMonoBold.ttf", settings.font_size);

    fontLarge.dropShadow(true);
    fontMedium.dropShadow(true);
    fontSmall.dropShadow(true);

    if(!balltex) balltex = texturemanager.grab("ball.tga");
    if(!glowtex) glowtex = texturemanager.grab("glow.tga");

    infowindow = TextArea(fontSmall);

    if(ipSummarizer != 0) delete ipSummarizer;

    total_space     = display.height - 40;
    remaining_space = total_space - 2;

    if(accesslog != 0) {
        delete accesslog;
        accesslog = 0;
    }

    ipSummarizer = new Summarizer(fontSmall, 100, settings.ip_summarizer_depth, 2.0f);
    ipSummarizer->addDelimiter(':');
    ipSummarizer->addDelimiter('.');
    ipSummarizer->setSize(2, 40, 0);

    for(Summarizer* s : summarizers) {
        delete s;
    }
    summarizers.clear();
    summarizer_types.clear();

    for(const std::string& group : settings.groups) {
        addGroup(group);
    }

    //add default groups
    if(summarizers.empty()) {
        //images - file is under images or
        addGroup("URI", "CSS", "(?i)\\.css\\b", 15);
        addGroup("URI", "Script", "(?i)\\.js\\b", 15);
        addGroup("URI", "Images", "(?i)/images/|\\.(jpe?g|gif|bmp|tga|ico|png)\\b", 20);
    }

    //always fill remaining space with Misc, (if there is some)
    if(remaining_space > 50) {
        addGroup("URI", "Misc", ".*");
    }

    reset();

    readLog();

    resizeSummarizers();

    //set start position
    if(settings.start_position > 0.0 && settings.start_position < 1.0) {
        seekTo(settings.start_position);
    }

    if(detect_changes) {
        if(config_watcher != 0) delete config_watcher;

        config_watcher = new ConfigWatcher();
        config_watcher->setConfig(settings.load_config);

        if(!config_watcher->start()) {
            delete config_watcher;
            config_watcher = 0;
        }
    }

    if(!initialized) {

        SDL_ShowCursor(false);

        // show slider so user knows its there unless recording
        if(hasProgressBar() && !frameExporter) {
            slider.show();
        }
    }

    initialized = true;
}

void Logstalgia::toggleFullscreen() {

    if(frameExporter != 0) return;

    texturemanager.unload();
    shadermanager.unload();
    fontmanager.unload();

    //recreate gl context
    display.toggleFullscreen();

    texturemanager.reload();
    shadermanager.reload();
    fontmanager.reload();

    reposition();
}

void Logstalgia::resize(int width, int height) {

    texturemanager.unload();
    shadermanager.unload();
    fontmanager.unload();

    display.resize(width, height);

    texturemanager.reload();
    shadermanager.reload();
    fontmanager.reload();

    reposition();
}

void Logstalgia::toggleWindowFrame() {
#if SDL_VERSION_ATLEAST(2,0,0)
    if(display.isFullscreen()) return;
    if(frameExporter != 0) return;

    texturemanager.unload();
    shadermanager.unload();
    fontmanager.unload();

    display.toggleFrameless();

    texturemanager.reload();
    shadermanager.reload();
    fontmanager.reload();

    reposition();
#endif
}

void Logstalgia::reposition() {
    initPaddles();
    initRequestBalls();
    resizeSummarizers();
    slider.resize();
}

void Logstalgia::setFrameExporter(FrameExporter* exporter) {

    int fixed_framerate = settings.output_framerate;
    int video_framerate = fixed_framerate;

    this->framecount = 0;
    this->frameskip  = 0;

    //calculate appropriate tick rate for video frame rate
    while(fixed_framerate < 60) {
        fixed_framerate += video_framerate;
        this->frameskip++;
    }

    this->fixed_tick_rate = 1.0f / ((float) fixed_framerate);

    this->frameExporter = exporter;
}

void Logstalgia::update(float t, float dt) {

    //if exporting a video use a fixed tick rate rather than time based
    if(frameExporter != 0) {
        dt = fixed_tick_rate;
    }

    //have to manage runtime internally as we're messing with dt
    runtime += dt;

    logic(runtime, dt);
    draw(runtime, dt);

    //extract frames based on frameskip setting
    //if frameExporter defined
    if(frameExporter != 0) {
        if(framecount % (frameskip+1) == 0) {
            frameExporter->dump();
        }
    }

   framecount++;
}

RequestBall* Logstalgia::findNearest(Paddle* paddle, const std::string& paddle_token) {

    float min_arrival = -1.0f;
    RequestBall* nearest = 0;

    for(RequestBall* ball: balls) {

        LogEntry* le = ball->getLogEntry();

        //special case if failed response code
        if(!le->successful) {
            continue;
        }

        if(le->successful && !ball->hasBounced()
            && (   (settings.paddle_mode <= PADDLE_SINGLE)
                || (settings.paddle_mode == PADDLE_VHOST && le->vhost == paddle_token)
                || (settings.paddle_mode == PADDLE_PID   && le->pid   == paddle_token)
               )
            ) {

            float arrival = ball->arrivalTime();

            if(min_arrival<0.0f || arrival<min_arrival) {
                min_arrival = arrival;
                nearest = ball;
            }
        }
    }

    return nearest;
}

void Logstalgia::removeBall(RequestBall* ball) {

    LogEntry* le = ball->getLogEntry();

    std::string url  = le->path;
    std::string host = le->hostname;

    for(Summarizer* s: summarizers) {
        if(s->supportedString(url)) {

            if(settings.hide_url_prefix) url = filterURLHostname(url);
            s->removeString(url);
            break;
        }
    }

    ipSummarizer->removeString(host);

    delete ball;
}

void Logstalgia::logic(float t, float dt) {

    float sdt = dt * settings.simulation_speed;

    //increment clock
    elapsed_time += sdt;
    currtime = starttime + (long)(elapsed_time);

    if(settings.stop_time && currtime > settings.stop_time) {
        currtime = settings.stop_time;
    }

    if(mousehide_timeout>0.0f) {
        mousehide_timeout -= dt;
        if(mousehide_timeout<0.0f) {
            SDL_ShowCursor(false);
        }
    }

    infowindow.hide();

    if(end_reached && balls.empty()) {
        appFinished = true;
        return;
    }

    if(!mouseOverSummarizerWidthAdjuster(mousepos)) {

        // update info window

        if(paused) {

            for(auto& it: paddles) {
                Paddle* paddle = it.second;

                if(paddle->mouseOver(infowindow, mousepos)) {
                    break;
                }
            }

            for(RequestBall* ball : balls) {
                if(ball->mouseOver(infowindow, mousepos)) {
                    break;
                }
            }
        }

        // inspect summarizer detail

        if(!ipSummarizer->getInfoAtPos(infowindow,mousepos)) {
            for(Summarizer* s: summarizers) {
                if(s->getInfoAtPos(infowindow, mousepos)) break;
            }
        }
    }

    if(paused) return;

    //next will fast forward clock to the time of the next entry,
    //if the next entry is in the future
    if(next || (!settings.disable_auto_skip && balls.empty())) {
        if(!queued_entries.empty()) {
            LogEntry* le = queued_entries.front();

            long entrytime = le->timestamp;
            if(entrytime > currtime) {
                elapsed_time = entrytime - starttime;
                currtime = starttime + (long)(elapsed_time);
            }
        }
        next = false;
    }

    //recalc spawn speed each second by
    if(currtime != lasttime) {

        //dont bother reading the log if we dont need to
        if(queued_entries.empty() || queued_entries.back()->timestamp <= currtime) {
            readLog();
        }

        profile_start("determine new entries");

        int items_to_spawn=0;

        for(LogEntry* le : queued_entries) {

            if(le->timestamp > currtime) break;

            items_to_spawn++;

            addStrings(le);
        }

        profile_stop();

        //debugLog("items to spawn %d\n", items_to_spawn);

        if(items_to_spawn > 0) {

            profile_start("add new strings");

            //re-summarize
            ipSummarizer->summarize();

            for(Summarizer* s : summarizers) {
                s->summarize();
            }

            profile_stop();

            profile_start("add new entries");

            float item_offset = 1.0 / (float) (items_to_spawn);

            int item_no = 0;

            while(!queued_entries.empty()) {

                LogEntry* le = queued_entries.front();

                if(le->timestamp > currtime) break;

                float pos_offset   = item_offset * (float) item_no++;
                float start_offset = std::min(1.0f, pos_offset);

                addBall(le, start_offset);

                queued_entries.pop_front();
            }

        }

        //update date
        if(total_entries>0) {
            char datestr[256];
            char timestr[256];

            struct tm* timeinfo = localtime ( &currtime );
            strftime(datestr, 256, "%A, %B %d, %Y", timeinfo);
            strftime(timestr, 256, "%X", timeinfo);

            displaydate = datestr;
            displaytime = timestr;
        } else {
            displaydate = "";
            displaytime = "";
        }

        lasttime=currtime;

        profile_stop();
    } else {
        //do small reads per frame if we havent buffered the next second
        if(queued_entries.empty() || queued_entries.back()->timestamp <= currtime+1) {
            readLog(50);
        }
    }

    std::list<Paddle*> inactivePaddles;

    //update paddles
    for(auto& it: paddles) {

        std::string paddle_token = it.first;
        Paddle*           paddle = it.second;

        if(settings.paddle_mode > PADDLE_SINGLE && !paddle->moving() && !paddle->visible()) {

            bool token_match = false;

            //are there any requests that will match this paddle?
            for(RequestBall* ball : balls) {

                LogEntry* le = ball->getLogEntry();

                if(   (settings.paddle_mode == PADDLE_VHOST && le->vhost == paddle_token)
                   || (settings.paddle_mode == PADDLE_PID   && le->pid   == paddle_token)) {
                    token_match = true;
                    break;
                }
            }

            //mark this paddle for deletion, continue
            if(!token_match) {
                inactivePaddles.push_back(paddle);
                continue;
            }
        }

        // find nearest ball to this paddle
        if( (retarget || !paddle->getTarget())) {

            RequestBall* ball = findNearest(paddle, paddle_token);

            if(ball != 0) {
                paddle->setTarget(ball);
            } else if(!paddle->moving()) {
                paddle->setTarget(0);
            }
        }

        paddle->logic(sdt);
    }

    retarget = false;

    profile_start("check ball status");

    // NOTE: special handling for this iterator as items are being removed
    for(auto it = balls.begin(); it != balls.end();) {

        RequestBall* ball = *it;

        highscore += ball->logic(sdt);

        if(ball->isFinished()) {
            it = balls.erase(it);
            removeBall(ball);
        } else {
            it++;
        }
    }

    profile_stop();

    profile_start("ipSummarizer logic");
    ipSummarizer->logic(dt);
    profile_stop();

    profile_start("updateGroups logic");
    updateGroups(dt);
    profile_stop();


    screen_blank_elapsed += dt;

    if(screen_blank_elapsed-screen_blank_interval > screen_blank_period)
        screen_blank_elapsed = 0.0f;

    //update font alpha
    font_alpha = 1.0f;

    if(screen_blank_elapsed>screen_blank_interval) {
        font_alpha = std::min(1.0f, (float) fabs(1.0f - (screen_blank_elapsed-screen_blank_interval)/(screen_blank_period*0.5)));
        font_alpha *= font_alpha;
    }
}

void Logstalgia::addGroup(const std::string& groupstr) {

    std::vector<std::string> group_definition;
    Regex groupregex("^([^,]+),(?:(HOST|CODE|URI)=)?([^,]+),([^,]+)(?:,([^,]+))?$");
    groupregex.match(groupstr, &group_definition);

    vec3 colour(0.0f, 0.0f, 0.0f);

    if(group_definition.size()>=4) {
        std::string group_name  = group_definition[0];
        std::string group_type  = group_definition[1];
        std::string group_regex = group_definition[2];

        if(group_type.empty()) group_type = "URI";

        debugLog("group_name %s group_type %s group_regex %s", group_name.c_str(), group_type.c_str(), group_regex.c_str());

        int percent = atoi(group_definition[3].c_str());

        // TODO: allow ommiting percent, if percent == 0, divide up remaining space amoung groups with no percent

        //check for optional colour param
        if(group_definition.size()>=5) {
            int col;
            int r, g, b;
            if(sscanf(group_definition[4].c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                colour = vec3( r, g, b );
                debugLog("r = %d, g = %d, b = %d\n", r, g, b);
                colour /= 255.0f;
            }
        }

        addGroup(group_type, group_name, group_regex, percent, colour);
    }
}

void Logstalgia::addGroup(const std::string& group_by, const std::string& grouptitle, const std::string& groupregex, int percent, vec3 colour) {

    if(percent<0) return;

    int remaining_percent = (int) ( ((float) remaining_space/total_space) * 100);

    if(remaining_percent<=0) return;

    if(!percent || percent > remaining_percent) {
        percent = remaining_percent;
    }


    Summarizer* summarizer = 0;

    try {
        summarizer = new Summarizer(fontSmall, percent, settings.group_summarizer_depth, settings.update_rate, groupregex, grouptitle);
        summarizer->addDelimiter('/');
    }
    catch(RegexCompilationException& e) {
        throw SDLAppException("invalid regular expression for group '%s'", grouptitle.c_str());
    }

    if(glm::dot(colour, colour) > 0.01f) {
        summarizer->setColour(colour);
    }

    if(!summarizer_types[group_by]) {
        summarizer_types[group_by] = new std::vector<Summarizer*>();
    }

    summarizers.push_back(summarizer);
    summarizer_types[group_by]->push_back(summarizer);

    int space = (int) ( ((float)percent/100) * total_space );
    remaining_space -= space;
}

void Logstalgia::resizeSummarizers() {

    ipSummarizer->setSize(2, 40, 0);

    total_space = display.height - 40;
    remaining_space = total_space - 2;

    for(Summarizer* s : summarizers) {

        int percent = s->getScreenPercent();

        // TODO: do something here ? flag as hidden ?
        //group->setVisible( remaining_percent >= percent );

        int top_gap    = total_space - remaining_space;

        int space = (int) ( ((float)percent/100) * total_space );
        int bottom_gap = display.height - (total_space - remaining_space + space);

        s->setSize(paddle_x, top_gap, bottom_gap);

        remaining_space -= space;
    }
}


void Logstalgia::updateGroups(float dt) {

    for(Summarizer* s : summarizers) {
        s->logic(dt);
    }
}

void Logstalgia::drawGroups(float dt, float alpha) {

    for(Summarizer* s : summarizers) {
        s->draw(dt, alpha);
    }
}

void Logstalgia::draw(float t, float dt) {
    if(appFinished) return;

    if(config_watcher != 0) {
        if(config_watcher->changeDetected()) {
            debugLog("config file modified");
            reloadConfig();
        }
        // perform check after to add a delay between
        // the file being modified and being reloaded
        config_watcher->update();
    }

    if(hasProgressBar()) slider.logic(dt);

    display.setClearColour(settings.background_colour);
    display.clear();

    glDisable(GL_FOG);

    display.mode2D();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    profile_start("draw ip summarizer");

    ipSummarizer->draw(dt, font_alpha);

    profile_stop();


    profile_start("draw groups");

    drawGroups(dt, font_alpha);

    profile_stop();


    profile_start("draw balls");

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, balltex->textureid);

    for(RequestBall* ball : balls) {
        ball->draw();
    }

    profile_stop();

    profile_start("draw response codes");

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
        RequestBall* r = *it;

        if(!settings.hide_response_code && r->hasBounced()) {
            r->drawResponseCode(&fontMedium);
        }
    }

    profile_stop();

    glDisable(GL_TEXTURE_2D);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if(settings.paddle_mode != PADDLE_NONE) {

        //draw paddles shadows
        for(auto& it: paddles) {
            it.second->drawShadow();
        }

        //draw paddles
        for(auto& it: paddles) {
            it.second->draw();
        }
    }

    if(settings.paddle_mode > PADDLE_SINGLE && !settings.hide_paddle_tokens) {

        glEnable(GL_TEXTURE_2D);

        //draw paddle tokens
        for(auto& it: paddles) {
            it.second->drawToken();
        }
    }

    if(!settings.disable_glow) {

        glBlendFunc (GL_ONE, GL_ONE);

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, glowtex->textureid);

        for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
            (*it)->drawGlow();
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    if(settings.splash > 0.0f) {
        int logowidth = fontLarge.getWidth("Logstalgia");
        int logoheight = 105;
        int cwidth    = fontMedium.getWidth("Website Access Log Viewer");
        int awidth    = fontMedium.getWidth("(C) 2008 Andrew Caudwell");

        vec2 corner(display.width/2 - logowidth/2 - 30.0f,
                     display.height/2 - 45);

        float logo_alpha = std::min(1.0f, settings.splash/3.0f);
        float logo_bg    = std::min(0.2f, settings.splash/10.0f);

        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.5f, 1.0f, logo_bg);
        glBegin(GL_QUADS);
            glVertex2f(0.0f,                 corner.y);
            glVertex2f(0.0f,                 corner.y + logoheight);
            glVertex2f(display.width, corner.y + logoheight);
            glVertex2f(display.width, corner.y);
        glEnd();

        glEnable(GL_TEXTURE_2D);

        fontLarge.alignTop(true);
        fontLarge.dropShadow(true);

        fontLarge.setColour(vec4(1.0f,1.0f,1.0f,logo_alpha));
        fontLarge.draw(display.width/2 - logowidth/2,display.height/2 - 30, "Logstalgia");
        fontLarge.setColour(vec4(0.0f,1.0f,1.0f,logo_alpha));
        fontLarge.draw(display.width/2 - logowidth/2,display.height/2 - 30, "Log");

        fontMedium.setColour(vec4(1.0f,1.0f,1.0f,logo_alpha));
        fontMedium.draw(display.width/2 - cwidth/2,display.height/2 + 17, "Website Access Log Viewer");
        fontMedium.draw(display.width/2 - awidth/2,display.height/2 + 37, "(C) 2008 Andrew Caudwell");

        settings.splash -= dt;
    }

    fontMedium.setColour(vec4(1.0f,1.0f,1.0f,font_alpha));

    if(info) {
        fontMedium.print(2,2, "FPS %d", (int) fps);
        fontMedium.print(2,19,"Balls: %d", balls.size());
        fontMedium.print(2,36,"Queue: %d", queued_entries.size());
        fontMedium.print(2,53,"Paddles: %d", paddles.size());
        fontMedium.print(2,70,"Simulation Speed: %.2f", settings.simulation_speed);
        fontMedium.print(2,87,"Pitch Speed: %.2f", settings.pitch_speed);
    } else {
        fontMedium.draw(2,2,  displaydate.c_str());
        fontMedium.draw(2,19, displaytime.c_str());
    }

    fontLarge.setColour(vec4(1.0f,1.0f,1.0f,font_alpha));

    int counter_width = fontLarge.getWidth("00000000");

    fontLarge.alignTop(false);

    fontLarge.print(display.width-10-counter_width,display.height-10, "%08d", highscore);

    infowindow.draw();

    if(hasProgressBar()) slider.draw(dt);

    if(take_screenshot) {
        screenshot();
        take_screenshot = false;
    }

    if(!message.empty() && message_timer > 0.0f) {

        fontMedium.setColour(vec4(1.0));
        int mwidth = fontMedium.getWidth(message);

        fontMedium.draw(display.width/2 - mwidth/2, display.height - fontMedium.getMaxHeight() - 2, message);
        message_timer -= dt;
    }
}
