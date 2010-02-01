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

#ifndef LOGSTALGIA_H
#define LOGSTALGIA_H

#define LOGSTALGIA_VERSION "0.9.5"

#ifdef _WIN32
#include "windows.h"
#endif

#include "core/display.h"
#include "core/fxfont.h"
#include "core/stringhash.h"
#include "core/seeklog.h"

#include "apache.h"
#include "custom.h"
#include "logentry.h"
#include "paddle.h"
#include "requestball.h"
#include "summarizer.h"
#include "textarea.h"
#include "slider.h"
#include "ppm.h"

#include <unistd.h>

#include <dirent.h>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

extern int   gHighscore;
extern bool  gBounce;
extern bool  gPaddle;
extern bool  gResponseCode;
extern bool  gDisableProgress;
extern float gSplash;
extern float gStartPosition;
extern float gStopPosition;

void logstalgia_help();
void logstalgia_info(std::string msg);
void logstalgia_quit(std::string error);
void logstalgia_help(std::string error);

class Logstalgia : public SDLApp {

    Paddle* paddle;

    int buffer_row_count;

    std::string logfile;

    std::string displaydate;
    std::string displaytime;

    bool info;
    bool paused;
    bool recentre;
    bool next;

    long starttime;
    long currtime;
    long lasttime;

    float elapsed_time;

    float simu_speed;
    float update_rate;

    float spawn_delay;
    float spawn_speed;

    std::string uimessage;
    float uimessage_timer;

    int total_space;
    int remaining_space;
    int total_entries;

    TextureResource* balltex;
    TextureResource* glowtex;

    float mousehide_timeout;
    vec2f mousepos;
    RequestBall* paddle_target;

    FXFont fontSmall;
    FXFont fontMedium;
    FXFont fontLarge;

    Summarizer* ipSummarizer;

    std::vector<Summarizer*> summGroups;

    PositionSlider slider;

    AccessLog* accesslog;

    SeekLog* seeklog;
    StreamLog* streamlog;

    std::deque<LogEntry> entries;
    std::list<RequestBall*> balls;

    TextArea infowindow;

    float time_scale;

    float runtime;
    float fixed_tick_rate;
    int framecount;
    int frameskip;
    FrameExporter* frameExporter;

    std::string dateAtPosition(float percent);
    void seekTo(float percent);

    void readLog();

    RequestBall* findNearest();
    void updateGroups(float dt);
    void drawGroups(float dt);

    void addBall(LogEntry& le,  float head_start);
    void removeBall(RequestBall* ball);
    void addGroup(std::string grouptitle, std::string groupregex, int percent = 0, vec3f colour = vec3f(0.0f, 0.0f, 0.0f));
    void togglePause();

    BaseLog* getLog();

    void reset();
	void logic(float t, float dt);
	void draw(float t, float dt);
public:
	Logstalgia(std::string logfile, float simu_speed, float update_rate);
	~Logstalgia();

    void addGroup(std::string groupstr);

    void setFrameExporter(FrameExporter* exporter, int video_framerate);

    //inherited methods
    void init();
    void update(float t, float dt);
	void keyPress(SDL_KeyboardEvent *e);
	void mouseMove(SDL_MouseMotionEvent *e);
	void mouseClick(SDL_MouseButtonEvent *e);
};

#endif
