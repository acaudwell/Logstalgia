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

//Logstalgia

//turn performance profiling
//#define LS_PERFORMANCE_PROFILE

float gSplash = -1.0f;
float gStartPosition = 0.0;
float gStopPosition  = 1.0;
float gPaddlePosition = 0.67;
bool  gAutoSkip = true;
int   gFontSize = 14;
bool  gDisableProgress = false;
bool  gSyncLog         = false;
bool  gHideURLPrefix   = false;

std::string profile_name;
Uint32 profile_start_msec;

std::string old_tz;

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

void logstalgia_help() {

#ifdef _WIN32
    SDLAppCreateWindowsConsole();

    //resize window to fit help message
    SDLAppResizeWindowsConsole(700);
#endif

    printf("Logstalgia v%s\n", LOGSTALGIA_VERSION);

    printf("Usage: logstalgia [OPTIONS] FILE\n\n");
    printf("Options:\n");
    printf("  -WIDTHxHEIGHT              Set window size\n");
    printf("  -f                         Fullscreen\n\n");

    printf("  -b --background FFFFFF     Background colour in hex\n\n");

    printf("  -x --full-hostnames        Show full request ip/hostname\n");
    printf("  -s --speed                 Simulation speed (default: 1)\n");
    printf("  -u --update-rate           Page summary update rate (default: 5)\n\n");
    printf("  -g name,regex,percent[,colour]  Group urls that match a regular expression\n\n");

    printf("  --paddle-mode MODE         Paddle mode (single, pid, vhost)\n");
    printf("  --paddle-position POSITION Paddle position as a fraction of the view width\n\n");

    printf("  --sync                     Read from STDIN, ignoring entries before now\n");
    printf("  --start-position POSITION  Begin at some position in the log (0.0 - 1.0)\n");
    printf("  --stop-position  POSITION  Stop at some position\n\n");

    printf("  --no-bounce                No bouncing\n\n");

    printf("  --hide-response-code       Hide response code\n");
    printf("  --hide-paddle              Hide paddle\n");
    printf("  --hide-url-prefix          Hide URL protocol and hostname prefix\n\n");
    printf("  --disable-auto-skip        Disable skipping of empty time periods\n");
    printf("  --disable-progress         Disable the progress bar\n");
    printf("  --disable-glow             Disable the glow effect\n\n");

    printf("  --font-size SIZE           Font size\n\n");

    printf("  --glow-duration            Duration of the glow (default: 0.15)\n");
    printf("  --glow-multiplier          Adjust the amount of glow (default: 1.25)\n");
    printf("  --glow-intensity           Intensity of the glow (default: 0.5)\n\n");

    printf("  --output-ppm-stream FILE Write frames as PPM to a file ('-' for STDOUT)\n");
    printf("  --output-framerate FPS   Framerate of output (25,30,60)\n\n");

    printf("FILE should be a log file or '-' to read STDIN.\n\n");

#ifdef _WIN32
    printf("Press Enter\n");
    getchar();
#endif

    exit(0);
}

Logstalgia::Logstalgia(std::string logfile, float simu_speed, float update_rate) : SDLApp() {
    info       = false;
    paused     = false;
    recentre   = false;
    next       = false;

    this->simu_speed  = simu_speed;
    this->update_rate = update_rate;

    this->logfile = logfile;

    spawn_delay=0;

    highscore = 0;

    uimessage_timer=0.0f;

    ipSummarizer  = 0;

    mintime       = gSyncLog ? time(0) : 0;
    seeklog       = 0;
    streamlog     = 0;

    if(!logfile.size())
        throw SDLAppException("no file supplied");

    if(logfile.compare("-")==0) {

        logfile = "STDIN";
        streamlog = new StreamLog();
        gDisableProgress = true;

    } else {
        try {
            seeklog = new SeekLog(logfile);

        } catch(SeekLogException& exception) {
            throw SDLAppException("unable to read log file");
        }
    }

    total_space = display.height - 40;
    remaining_space = total_space - 2;

    total_entries=0;

    background = vec3f(0.0, 0.0, 0.0);

    fontLarge  = fontmanager.grab("FreeSerif.ttf", 42);
    fontMedium = fontmanager.grab("FreeMonoBold.ttf", 16);
    fontBall   = fontmanager.grab("FreeMonoBold.ttf", 16);
    fontSmall  = fontmanager.grab("FreeMonoBold.ttf", gFontSize);

    fontLarge.dropShadow(true);
    fontMedium.dropShadow(true);
    fontSmall.dropShadow(true);

    balltex  = texturemanager.grab("ball.tga");
    glowtex = texturemanager.grab("glow.tga");

    infowindow = TextArea(fontSmall);

    mousehide_timeout = 0.0f;

    time_scale = 1.0;

    runtime = 0.0;
    frameExporter = 0;
    framecount = 0;
    frameskip = 0;
    fixed_tick_rate = 0.0;

    accesslog = 0;

    font_alpha = 1.0;

    //every 60 minutes seconds blank text for 60 seconds

    screen_blank_interval = 3600.0;
    screen_blank_period   = 60.0;
    screen_blank_elapsed  = 0.0;

    paddle_x = display.width * gPaddlePosition;
    paddle_colour = (gPaddleMode > PADDLE_SINGLE) ?
        vec4f(0.0f, 0.0f, 0.0f, 0.0f) : vec4f(0.5, 0.5, 0.5, 1.0);

    debugLog("Logstalgia end of constructor\n");

    //check if TZ is set, store current value
    if(old_tz.empty()) {
        char* current_tz_env = getenv("TZ");

        if(current_tz_env != 0) {
            old_tz  = std::string("TZ=");
            old_tz += std::string(current_tz_env);
        }
    }
}

Logstalgia::~Logstalgia() {
    if(accesslog!=0) delete accesslog;

    for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {
        delete it->second;
    }
    paddles.clear();

    if(seeklog!=0) delete seeklog;
    if(streamlog!=0) delete streamlog;

    for(size_t i=0;i<summGroups.size();i++) {
        delete summGroups[i];
        summGroups[i]=0;
    }

}

void Logstalgia::togglePause() {
    paused = !paused;

    if(!paused) {
        ipSummarizer->mouseOut();

        int nogrps = summGroups.size();
        for(int i=0;i<nogrps;i++) {
            summGroups[i]->mouseOut();
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
            gSplash = 10.0f;
        }

        if(e->keysym.sym == SDLK_n) {
            next = true;
        }

        if(e->keysym.sym == SDLK_SPACE) {
            togglePause();
        }

        if(e->keysym.sym == SDLK_EQUALS || e->keysym.sym == SDLK_KP_PLUS) {
            if(simu_speed<=29.0f) {
                simu_speed += 1.0f;
                recentre=true;
            }
        }

        if(e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_MINUS) {
            if(simu_speed>=2.0f) {
                simu_speed -= 1.0f;
                recentre=true;
            }
        }

        if(e->keysym.sym == SDLK_PERIOD) {

            if(time_scale>=1.0) {
                time_scale = std::min(4.0f, floorf(time_scale) + 1.0f);
            } else {
                time_scale = std::min(1.0f, time_scale * 2.0f);
            }
        }

        if(e->keysym.sym == SDLK_COMMA) {

            if(time_scale>1.0) {
                time_scale = std::max(0.0f, floorf(time_scale) - 1.0f);
            } else {
                time_scale = std::max(0.25f, time_scale * 0.5f);
            }
        }

    }
}

void Logstalgia::reset() {

    end_reached = false;

    highscore = 0;

    for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {
        delete it->second;
    }
    paddles.clear();

    if(gPaddleMode <= PADDLE_SINGLE) {
        vec2f paddle_pos = vec2f(paddle_x - 20, rand() % display.height);
        Paddle* paddle = new Paddle(paddle_pos, paddle_colour, "");
        paddles[""] = paddle;
    }

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
        removeBall(*it);
    }

    balls.clear();

    ipSummarizer->recalc_display();

    for(size_t i=0;i<summGroups.size();i++) {
        summGroups[i]->recalc_display();
    }

    queued_entries.clear();

    // reset settings
    elapsed_time  = 0;
    starttime     = 0;
    lasttime      = 0;
}

void Logstalgia::seekTo(float percent) {
    debugLog("seekTo(%.2f)\n", percent);

    if(gDisableProgress) return;

    //disable pause if enabled before seeking
    if(paused) paused = false;

    reset();

    seeklog->seekTo(percent);

    readLog();
}

void Logstalgia::mouseClick(SDL_MouseButtonEvent *e) {
    debugLog("click! (x=%d,y=%d)\n", e->x, e->y);

    if(e->type != SDL_MOUSEBUTTONDOWN) return;

    if(e->button == SDL_BUTTON_LEFT) {

        if(!gDisableProgress) {
            float position;
            if(slider.click(mousepos, &position)) {
                seekTo(position);
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

        if(accesslog->parseLine(linestr, le)) {

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

void Logstalgia::mouseMove(SDL_MouseMotionEvent *e) {
    mousepos = vec2f(e->x, e->y);
    SDL_ShowCursor(true);
    mousehide_timeout = 5.0f;

    float pos;

    if(!gDisableProgress && slider.mouseOver(mousepos, &pos)) {
        std::string date = dateAtPosition(pos);
        slider.setCaption(date);
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

void Logstalgia::addStrings(LogEntry* le) {

    std::string hostname = le->hostname;
    std::string pageurl  = le->path;

    int nogroups = summGroups.size();
    Summarizer* pageSummarizer= 0;

    for(int i=0;i<nogroups;i++) {
        if(summGroups[i]->supportedString(pageurl)) {
            pageSummarizer = summGroups[i];
            break;
        }
    }

    if(pageSummarizer==0) return;

    if(gHideURLPrefix) pageurl = filterURLHostname(pageurl);

    pageSummarizer->addString(pageurl);
    ipSummarizer->addString(hostname);
}

void Logstalgia::addBall(LogEntry* le, float start_offset) {

    std::string hostname = le->hostname;
    std::string pageurl  = le->path;

    //find appropriate summarizer for url
    int nogroups = summGroups.size();
    Summarizer* pageSummarizer= 0;

    for(int i=0;i<nogroups;i++) {
        if(summGroups[i]->supportedString(pageurl)) {
            pageSummarizer = summGroups[i];
            break;
        }
    }

    if(pageSummarizer==0) return;

    Paddle* entry_paddle = 0;

    if(gPaddleMode > PADDLE_SINGLE) {

        std::string paddle_token = (gPaddleMode == PADDLE_VHOST) ? le->vhost : le->pid;

        entry_paddle = paddles[paddle_token];

        if(entry_paddle == 0) {
            vec2f paddle_pos = vec2f(display.width-(display.width/3), rand() % display.height);
            Paddle* paddle = new Paddle(paddle_pos, paddle_colour, paddle_token);
            entry_paddle = paddles[paddle_token] = paddle;
        }

    } else {
        entry_paddle = paddles[""];
    }

    if(gHideURLPrefix) pageurl = filterURLHostname(pageurl);

    float dest_y = pageSummarizer->getMiddlePosY(pageurl);
    float pos_y  = ipSummarizer->getMiddlePosY(hostname);

    float start_x = -(entry_paddle->getX()/ 5.0f);

    vec2f ball_start = vec2f(start_x, pos_y);
    vec2f ball_dest  = vec2f(entry_paddle->getX(), dest_y);

    const std::string& match = ipSummarizer->getBestMatchStr(hostname);

    vec3f colour = pageSummarizer->isColoured() ? pageSummarizer->getColour() : colourHash(match);

    RequestBall* ball = new RequestBall(le, &fontMedium, balltex, colour, ball_start, ball_dest, simu_speed);

    ball->setElapsed( start_offset );

    balls.push_back(ball);
}

BaseLog* Logstalgia::getLog() {
    if(seeklog !=0) return seeklog;

    return streamlog;
}

void Logstalgia::readLog(int buffer_rows) {

    profile_start("readLog");

    //change TZ to UTC
    putenv((char*)"TZ=UTC");
    tzset();

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

            if(mintime == 0 || mintime <= le.timestamp) {

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

    //reset TZ to previous value

    if(!old_tz.empty()) {
        putenv((char*)old_tz.c_str());
    } else {
#ifdef HAVE_UNSETENV
        unsetenv("TZ");
#else
        putenv("TZ=");
#endif
    }

    tzset();

    if(queued_entries.empty() && seeklog != 0) {

        if(total_entries==0) {
            logstalgia_quit("could not parse first entry");
        }

        //no more entries
        end_reached = true;

        return;
    }

    if(seeklog != 0) {
        float percent = seeklog->getPercent();

        if(percent > gStopPosition) {
            end_reached = true;
            return;
        }

        if(!gDisableProgress) slider.setPercent(percent);
    }

    //set start time if currently 0
    if(starttime==0 && !queued_entries.empty()) {
        starttime = queued_entries.front()->timestamp;
        currtime  = 0;
    }
}

void Logstalgia::init() {
    debugLog("init called\n");

    ipSummarizer = new Summarizer(fontSmall, 2, 40, 0, 2.0f);

    reset();

    readLog();

    //add default groups
    if(summGroups.size()==0) {
        //images - file is under images or
        addGroup("CSS", "\\.css\\b", 15);
        addGroup("Script", "\\.js\\b", 15);
        addGroup("Images", "/images/|\\.(jpe?g|gif|bmp|tga|ico|png)\\b", 20);
    }

    //always fill remaining space with Misc, (if there is some)
    if(remaining_space>50) {
        addGroup(summGroups.size()>0 ? "Misc" : "", ".*");
    }

    SDL_ShowCursor(false);

    //set start position
    if(gStartPosition > 0.0 && gStartPosition < 1.0) {
        seekTo(gStartPosition);
    }
}

void Logstalgia::setBackground(vec3f background) {
    this->background = background;
}

void Logstalgia::setFrameExporter(FrameExporter* exporter, int video_framerate) {

    int fixed_framerate = video_framerate;

    this->framecount = 0;
    this->frameskip  = 0;

    //calculate appropriate tick rate for video frame rate
    while(fixed_framerate<60) {
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

    dt *= time_scale;

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

    float min_dist = -1.0f;
    RequestBall* nearest = 0;

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
        RequestBall* ball = *it;

        //special case if failed response code
        if(!ball->le->successful) {
            continue;
        }

        if(ball->le->successful && !ball->hasBounced()
            && (gPaddleMode <= PADDLE_SINGLE
                || gPaddleMode == PADDLE_VHOST && ball->le->vhost == paddle_token
                || gPaddleMode == PADDLE_PID   && ball->le->pid   == paddle_token
               )
            ) {
            float dist = (paddle->getX() - ball->getX())/ball->speed;
            if(min_dist<0.0f || dist<min_dist) {
                min_dist = dist;
                nearest = ball;
            }
        }
    }

    return nearest;
}

void Logstalgia::removeBall(RequestBall* ball) {
    std::string url  = ball->le->path;
    std::string host = ball->le->hostname;

    int nogroups = summGroups.size();

    for(int i=0;i<nogroups;i++) {
        if(summGroups[i]->supportedString(url)) {

            if(gHideURLPrefix) url = filterURLHostname(url);

            summGroups[i]->removeString(url);
            break;
        }
    }

    ipSummarizer->removeString(host);

    delete ball;
}

void Logstalgia::logic(float t, float dt) {

    float sdt = dt*simu_speed;;


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

    //if paused, dont move anything, only check what is under mouse
    if(paused) {

        for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {
            std::string paddle_token = it->first;
            Paddle*           paddle = it->second;

            if(paddle->mouseOver(infowindow, mousepos)) {
                break;
            }
        }

        for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
            RequestBall* ball = *it;
            if(ball->mouseOver(infowindow, mousepos)) {
                break;
            }
        }

        if(!ipSummarizer->mouseOver(infowindow,mousepos)) {
            int nogrps = summGroups.size();
            for(int i=0;i<nogrps;i++) {
                if(summGroups[i]->mouseOver(infowindow, mousepos)) break;
            }
        }

        return;
    }

    //increment clock
    elapsed_time += sdt;
    currtime = starttime + (long)(elapsed_time);

    //next will fast forward clock to the time of the next entry, 
    //if the next entry is in the future
    if(next || gAutoSkip && balls.empty()) {
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

        for(std::list<LogEntry*>::iterator it = queued_entries.begin(); it != queued_entries.end(); it++) {
            LogEntry* le = *it;

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

            int nogrps = summGroups.size();

            for(int i=0;i<nogrps;i++) {
                summGroups[i]->summarize();
            }

            profile_stop();

            profile_start("add new entries");

            float item_offset = 1.0 / (float) (items_to_spawn);

            int item_no = 0;
            
            while(!queued_entries.empty()) {

                LogEntry* le = queued_entries.front();

                if(le->timestamp > currtime) break;

                float pos_offset   = 1.0 - item_offset * (float) item_no++;
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
    for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {

        std::string paddle_token = it->first;
        Paddle*           paddle = it->second;

        if(gPaddleMode > PADDLE_SINGLE && !paddle->moving() && !paddle->visible()) {

            bool token_match = false;

            //are there any requests that will match this paddle?
            for(std::list<RequestBall*>::iterator bit = balls.begin(); bit != balls.end();bit++) {

                RequestBall* ball = *bit;

                if(   gPaddleMode == PADDLE_VHOST && ball->le->vhost == paddle_token
                   || gPaddleMode == PADDLE_PID   && ball->le->pid   == paddle_token) {
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
        if((recentre || !paddle->moving()) && balls.size()>0) {

            recentre=false;

            RequestBall* ball = findNearest(paddle, paddle_token);

            if(ball!=0 && !(paddle->moving() && paddle->getTarget() == ball)) {
                paddle->setTarget(ball);
            }

        }

        //if still not moving, recentre
        if(!paddle->moving()) {
            recentre=true;
            paddle->setTarget(0);
        }

        it->second->logic(sdt);
    }

    recentre = false;

    profile_start("check ball status");

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end();) {

        RequestBall* ball = *it;

        highscore += ball->logic(dt);

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

void Logstalgia::addGroup(std::string groupstr) {

    std::vector<std::string> groupdef;
    Regex groupregex("^([^,]+),([^,]+),([^,]+)(?:,([^,]+))?$");
    groupregex.match(groupstr, &groupdef);

    vec3f colour(0.0f, 0.0f, 0.0f);

    if(groupdef.size()>=3) {
        std::string groupname = groupdef[0];
        std::string groupregex = groupdef[1];
        int percent = atoi(groupdef[2].c_str());

        //check for optional colour param
        if(groupdef.size()>=4) {
            int col;
            int r, g, b;
            if(sscanf(groupdef[3].c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                colour = vec3f( r, g, b );
                debugLog("r = %d, g = %d, b = %d\n", r, g, b);
                colour /= 255.0f;
            }
        }

        addGroup(groupname, groupregex, percent, colour);
    }
}

void Logstalgia::addGroup(std::string grouptitle, std::string groupregex, int percent, vec3f colour) {

    if(percent<0) return;

    int remainpc = (int) ( ((float) remaining_space/total_space) * 100);

    if(percent==0) {
        percent=remainpc;
    }

    if(remainpc<percent) return;

    int space = (int) ( ((float)percent/100) * total_space );

    int top_gap    = total_space - remaining_space;
    int bottom_gap = display.height - (total_space - remaining_space + space);

    //debugLog("group %s: regex = %s, remainpc = %d, space = %d, top_gap = %d, bottom_gap = %d\n",
    //    grouptitle.c_str(), groupregex.c_str(), remainpc, space, top_gap, bottom_gap);

    Summarizer* summ = new Summarizer(fontSmall, paddle_x, top_gap, bottom_gap, update_rate, groupregex, grouptitle);
//    summ->showCount(true);

    if(colour.length2() > 0.01f) {
        summ->setColour(colour);
    }

    summGroups.push_back(summ);

    remaining_space -= space;
}

void Logstalgia::updateGroups(float dt) {

    int nogrps = summGroups.size();
    for(int i=0;i<nogrps;i++) {
        summGroups[i]->logic(dt);
    }

}

void Logstalgia::drawGroups(float dt, float alpha) {

    int nogrps = summGroups.size();
    for(int i=0;i<nogrps;i++) {
        summGroups[i]->draw(dt, alpha);
    }

}

void Logstalgia::draw(float t, float dt) {
    if(appFinished) return;

    if(!gDisableProgress) slider.logic(dt);

    display.setClearColour(background);
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

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
        (*it)->draw(dt);
    }

    profile_stop();

    profile_start("draw response codes");

    for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
        RequestBall* r = *it;

        if(gResponseCode && r->hasBounced()) {
            r->drawResponseCode();
        }
    }

    profile_stop();

    glDisable(GL_TEXTURE_2D);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if(gPaddleMode != PADDLE_NONE) {

        //draw paddles shadows
        for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {
            it->second->drawShadow();
        }

        //draw paddles
        for(std::map<std::string, Paddle*>::iterator it= paddles.begin(); it!=paddles.end();it++) {
            it->second->draw();
        }

    }

    if(!gDisableGlow) {

        glBlendFunc (GL_ONE, GL_ONE);

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, glowtex->textureid);

        for(std::list<RequestBall*>::iterator it = balls.begin(); it != balls.end(); it++) {
            (*it)->drawGlow();
        }
    }

    infowindow.draw();

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    if(uimessage_timer>0.1f) {
        glColor4f(1.0,1.0,uimessage_timer/3.0f,uimessage_timer/3.0f);

        int mwidth = fontLarge.getWidth(uimessage.c_str());

        fontLarge.draw(display.width/2 - mwidth/2, display.height/2 - 20, uimessage.c_str());
        uimessage_timer-=dt;
    }


    if(gSplash>0.0f) {
        int logowidth = fontLarge.getWidth("Logstalgia");
        int logoheight = 105;
        int cwidth    = fontMedium.getWidth("Website Access Log Viewer");
        int awidth    = fontMedium.getWidth("(C) 2008 Andrew Caudwell");

        vec2f corner(display.width/2 - logowidth/2 - 30.0f,
                     display.height/2 - 45);

        float logo_alpha = std::min(1.0f, gSplash/3.0f);
        float logo_bg    = std::min(0.2f, gSplash/10.0f);

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

        glColor4f(1,1,1,logo_alpha);
        fontLarge.draw(display.width/2 - logowidth/2,display.height/2 - 30, "Logstalgia");
        glColor4f(0,1,1,logo_alpha);
        fontLarge.draw(display.width/2 - logowidth/2,display.height/2 - 30, "Log");

        glColor4f(1,1,1,logo_alpha);
        fontMedium.draw(display.width/2 - cwidth/2,display.height/2 + 17, "Website Access Log Viewer");
        fontMedium.draw(display.width/2 - awidth/2,display.height/2 + 37, "(C) 2008 Andrew Caudwell");

        gSplash-=dt;
    }

    if(!gDisableProgress) slider.draw(dt);

    glColor4f(1,1,1,font_alpha);

    if(info) {
        fontMedium.print(2,2, "FPS %d", (int) fps);
        fontMedium.print(2,19,"Balls %03d", balls.size());
        fontMedium.print(2,36,"Queue %03d", queued_entries.size());
        fontMedium.print(2,53,"Paddles %03d", paddles.size());
    } else {
        fontMedium.draw(2,2,  displaydate.c_str());
        fontMedium.draw(2,19, displaytime.c_str());
    }
    glColor4f(1,1,1,font_alpha);

    int counter_width = fontLarge.getWidth("00000000");
    fontLarge.alignRight(false);
    fontLarge.alignTop(false);
    fontSmall.alignRight(true);
    fontSmall.alignTop(false);

    fontLarge.print(display.width-10-counter_width,display.height-10, "%08d", highscore);

    // draw avg. requests per second
    if (starttime && currtime) {
        int diff = currtime - starttime;
        if (diff > 0) {
            int requests = ceil(highscore / (currtime - starttime));
            fontSmall.print(display.width-10,display.height-45, "%d req/s", requests);
        }
    }

}
