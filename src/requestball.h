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

#include <vector>
#include <string>

#include "core/vectors.h"

class FXFont;
class TextArea;
class LogEntry;

class RequestBall {
protected:
    std::vector<vec2> points;
    std::vector<float> line_lengths;

    LogEntry* le;

    float size;

    vec2 pos;
    vec2 dest;
    vec2 dir;

    vec3 colour;

    float distance_travelled;
    float total_distance;

    bool has_bounced;
    bool no_bounce;

    vec2 offset;

    void formatRequestDetail(LogEntry *le, std::vector<std::string>& content);

    float getProgress() const;

    void project();
    void bounce();

    void addPoint(const vec2& p);

    void animate(float dt);
public:
    RequestBall(LogEntry* le, const vec3& colour, const vec2& pos, const vec2& dest);
    ~RequestBall();

    bool mouseOver(TextArea& textarea, vec2& mouse);

    float arrivalTime();

    bool isFinished() const;
    bool hasBounced() const;

    void changeDestX(float dest_x);

    const vec2& getFinishPos() const;

    const vec3& getColour() const;
    LogEntry* getLogEntry() const;

    int logic(float dt);

    void drawGlow() const;
    void draw() const;
    void drawResponseCode(FXFont* font) const;
};

#endif
