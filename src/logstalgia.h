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

#include "core/sdlapp.h"
#include "core/fxfont.h"
#include "core/seeklog.h"
#include "core/ppm.h"

#include "logentry.h"
#include "paddle.h"
#include "requestball.h"
#include "summarizer.h"
#include "textarea.h"
#include "slider.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <time.h>

class Logstalgia : public SDLApp {

    std::map<std::string,Paddle*> paddles;

    std::string logfile;

    std::string displaydate;
    std::string displaytime;

    bool info;
    bool paused;
    bool retarget;
    bool next;
    bool sync;
    bool end_reached;
    bool take_screenshot;

    int highscore;

    time_t mintime;

    time_t starttime;
    time_t currtime;
    time_t lasttime;

    float screen_blank_interval;
    float screen_blank_period;
    float screen_blank_elapsed;

    float font_alpha;

    double elapsed_time;

    float simu_speed;
    float update_rate;

    float spawn_delay;
    float spawn_speed;

    std::string message;
    float message_timer;

    int total_space;
    int remaining_space;
    int total_entries;

    vec3 background;
    vec4 paddle_colour;
    float paddle_x;

    TextureResource* balltex;
    TextureResource* glowtex;

    float mousehide_timeout;
    vec2 mousepos;
    RequestBall* paddle_target;

    FXFont fontSmall;
    FXFont fontMedium;
    FXFont fontLarge;
    FXFont fontBall;

    Summarizer* ipSummarizer;

    std::vector<Summarizer*> summarizers;
    std::map<std::string, std::vector<Summarizer*>*> summarizer_types;

    PositionSlider slider;

    AccessLog* accesslog;

    SeekLog* seeklog;
    StreamLog* streamlog;

    std::list<LogEntry*> queued_entries;
    std::list<RequestBall*> balls;

    TextArea infowindow;

    float time_scale;

    float runtime;
    float fixed_tick_rate;
    int framecount;
    int frameskip;
    FrameExporter* frameExporter;

    std::string filterURLHostname(const std::string& hostname);

    std::string dateAtPosition(float percent);
    void seekTo(float percent);

    void readLog(int buffer_rows = 0);

    RequestBall* findNearest(Paddle* paddle, const std::string& paddle_token);
    void updateGroups(float dt);
    void drawGroups(float dt, float alpha);

    Summarizer* getGroupSummarizer(LogEntry* le);

    void addStrings(LogEntry* le);

    void addBall(LogEntry* le,  float start_offset);
    void removeBall(RequestBall* ball);
    void addGroup(const std::string& group_by, const std::string& grouptitle, const std::string& groupregex, int percent = 0, vec3 colour = vec3(0.0f, 0.0f, 0.0f));
    void togglePause();

    BaseLog* getLog();

    void reset();

    void reinit();

    void initPaddles();
    void initRequestBalls();
    void resizeGroups();

    void setMessage(const char* str, ...);

    void screenshot();

    void toggleFullscreen();

    void logic(float t, float dt);
    void draw(float t, float dt);
public:
    Logstalgia(const std::string& logfile);
    ~Logstalgia();

    void addGroup(const std::string& groupstr);

    void setFrameExporter(FrameExporter* exporter);

    void setBackground(vec3 background);

    void resize(int width, int height);

    //inherited methods
    void init();
    void update(float t, float dt);
	void keyPress(SDL_KeyboardEvent *e);
	void mouseMove(SDL_MouseMotionEvent *e);
	void mouseClick(SDL_MouseButtonEvent *e);
};

#endif
