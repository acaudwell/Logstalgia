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

extern bool gPaddle;

class Paddle {

protected:
    vec2f pos;

    vec4f colour;
    vec4f lastcol;
    vec4f nextcol;

    int width, height;

    int   start_y;
    int   dest_y;
    float dest_eta;
    float dest_elapsed;

public:
    Paddle(vec2f pos, vec3f colour);
    ~Paddle();
    void moveTo(int y, float eta, vec3f nextcol);
    bool moving();
    void logic(float dt);
    void draw(float dt);
    float getX();
    float getY();
};

#endif
