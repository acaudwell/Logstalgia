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

#ifndef BALL_H
#define BALL_H

#include <vector>

#include "core/vectors.h"

class Line {

public:
    vec2 start;
    vec2 end;
    Line(vec2 start, vec2 end);
    ~Line();
    bool intersects(Line& l, vec2* p = 0);
};

class ProjectedBall {

protected:
    std::vector<vec2> points;
    std::vector<float> line_lengths;
    
    float distance_travelled;
    float total_distance;

    float dest_x;
    float start_x;
    
    bool has_bounced;
    bool no_bounce;
public:
    vec2 pos;
    vec2 vel;
    float size;
    vec3 colour;
    float speed;

    ProjectedBall();
    ProjectedBall(const vec2& pos, const vec2& vel, const vec3& colour, int dest_x, float size);
    ~ProjectedBall();

    void init(const vec2& pos, const vec2& vel, const vec3& colour, int dest_x, float size);

    void setOffset(float offset);
    
    void project();
    vec2 finish();

    void bounce();
    
    void dontBounce();

    float getX();

    bool isFinished() const;
    bool hasBounced() const { return has_bounced; }

    float arrivalTime();
    float getProgress() const;

    void logic(float dt);
};

#endif
