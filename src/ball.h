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
#include "core/sdlapp.h"

class Line {

public:
    vec2f start;
    vec2f end;
    Line(vec2f start, vec2f end);
    ~Line();
    bool intersects(Line& l, vec2f* p = 0);
};

class ProjectedBall {

protected:
    std::vector<vec2f> points;
    std::vector<float> line_lengths;
    float total_length;
    int dest_x;
    int start_x;
    float eta;
    float elapsed;
    float progress;
    bool has_bounced;
    bool no_bounce;

public:
    vec2f pos;
    vec2f vel;
    float size;
    vec3f colour;
    float speed;

    ProjectedBall();
    ProjectedBall(const vec2f& pos, const vec2f& vel, const vec3f& colour, int dest_x, float eta, float size, float speed = 10.0f);
    ~ProjectedBall();

    void init(const vec2f& pos, const vec2f& vel, const vec3f& colour, int dest_x, float eta, float size, float speed);

    void setElapsed(float e);
    void project();
    vec2f finish();

    void bounce();
    
    void dontBounce();

    float getX();

    bool isFinished() const;
    bool hasBounced() const { return has_bounced; }

    bool arrived() const;

    float arrivalTime();
    float getProgress() const;

    void logic(float dt);
};

#endif
