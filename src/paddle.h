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

#include "textarea.h"
#include "core/fxfont.h"
#include "core/vectors.h"

class RequestBall;
class Paddle {

protected:
    vec2 pos;

    RequestBall* target;

    std::string token;
    vec3 token_colour;

    vec4 default_colour;
    vec4 proc_colour;
    vec4 colour;
    vec4 lastcol;
    vec4 nextcol;

    int width, height;

    int   start_y;
    int   dest_y;
    float dest_eta;
    float dest_elapsed;

    FXFont font;
public:
    Paddle(vec2 pos, vec4 colour, std::string token, FXFont font);
    ~Paddle();
    void moveTo(int y, float eta, vec4 nextcol);
    bool moving();
    bool visible();

    void setTarget(RequestBall* target);
    RequestBall* getTarget();

    void logic(float dt);

    bool mouseOver(TextArea& textarea, vec2& mouse);

    void drawToken();
    void drawShadow();
    void draw();

    float getX();
    float getY();
};

#endif
