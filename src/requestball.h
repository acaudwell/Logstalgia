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

#ifndef REQUESTBALL_H
#define REQUESTBALL_H

#include "core/fxfont.h"

#include "logentry.h"
#include "ball.h"
#include "textarea.h"

class RequestBall : public ProjectedBall {
protected:

    vec2f start;
    vec2f dest;

    vec3f pagecolour;
    vec3f responseColour();

    FXFont* font;
    TextureResource* tex;

//    TextArea textarea;

    std::string response_code;
    vec3f       response_colour;
public:
    LogEntry* le;

    RequestBall(LogEntry* le, FXFont* font, TextureResource* tex, vec3f colour, vec2f pos, vec2f dest, float speed = 10.0f);
    ~RequestBall();

    bool mouseOver(TextArea& textarea, vec2f& mouse);

    int logic(float dt);

    void drawGlow() const;
    void draw(float dt) const;
    void drawResponseCode() const;
};

extern bool gDisableGlow;
extern float gGlowIntensity;
extern float gGlowMultiplier;
extern float gGlowDuration;


#endif
