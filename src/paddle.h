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

#ifndef PADDLE_H
#define PADDLE_H

#include "core/vectors.h"
#include "core/sdlapp.h"
#include "core/stringhash.h"
#include "core/fxfont.h"

#include "requestball.h"

#define PADDLE_NONE   0
#define PADDLE_SINGLE 1
#define PADDLE_PID    2
#define PADDLE_VHOST  3

extern int gPaddleMode;

class Paddle {

protected:
    vec2f pos;

    RequestBall* target;

    std::string token;
    vec3f token_colour;

    vec4f default_colour;
    vec4f proc_colour;
    vec4f colour;
    vec4f lastcol;
    vec4f nextcol;

    int width, height;

    int   start_y;
    int   dest_y;
    float dest_eta;
    float dest_elapsed;
    
    FXFont font;

public:
    Paddle(vec2f pos, vec4f colour, std::string token, FXFont font = 0);
    ~Paddle();
    void moveTo(int y, float eta, vec4f nextcol);
    bool moving();
    bool visible();

    void setTarget(RequestBall* target);
    RequestBall* getTarget();

    void logic(float dt);

    bool mouseOver(TextArea& textarea, vec2f& mouse);

    void drawShadow();
    void draw();

    float getX();
    float getY();
};

#endif
