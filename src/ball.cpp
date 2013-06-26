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

#include "ball.h"
#include "settings.h"

#include "core/display.h"

//Line
Line::Line(vec2 start, vec2 end) {
	this->start = start;
	this->end   = end;
}

Line::~Line() {
}

// check if line intersects the plane of another line

bool Line::intersects(Line& l, vec2 *p) {
    vec2 a = end   - start;
    vec2 b = l.end - l.start;

    float d = a.x*b.y - a.y * b.x;

    //parallel test
    if (!d) return false;

    vec2 w = start - l.start;

    //check if outside either line segment
    float s = b.x * w.y - b.y * w.x;

    float sd = s/d;
    if (sd < 0.0f || sd > 1.0f) return false;

    float t = a.x * w.y - a.y * w.x;

    float td = t/d;
    if (td < 0.0f || td > 1.0f) return false;

    //calculate intersect
    if(p != 0) *p = a * sd + start;

    return true;
}

//Projected Ball
ProjectedBall::ProjectedBall() {
}

ProjectedBall::ProjectedBall(const vec2& pos, const vec2& vel, const vec3& colour, int dest_x, float size) {
    init(pos, vel, colour, dest_x, size);
}

void ProjectedBall::init(const vec2& pos, const vec2& vel, const vec3& colour, int dest_x, float size) {

    this->pos = pos;
    this->vel = vel;
    this->colour = colour;
    this->size  = size;
    this->dest_x = dest_x;
    
    start_x = (int) pos.x;
    
    has_bounced=0;
    no_bounce = 0;

    project();
}


ProjectedBall::~ProjectedBall() {
}

/*project path of the ball
  as a series of line segments
*/

void ProjectedBall::project() {
    distance_travelled = 0.0f;
    total_distance     = 0.0f;

    points.clear();
    vec2 p = pos;
    points.push_back(p);

    //project current position and direction until it hits a wall or crosses dest_x
    Line finish(vec2(dest_x, 0), vec2(dest_x, display.height));
    Line top(vec2(0,0), vec2(display.width, 0));
    Line bottom(vec2(0,display.height), vec2(display.width, display.height));

    //project far enough to cross a side of the screen
    float inc = display.width*2.0f;

    bool finished=false;
    vec2 currvel = vel;

    line_lengths.clear();

    while(!finished) {
        Line proj(p, p + currvel*inc);

        /*
        debugLog("proj = (%.2f, %.2f), (%.2f, %.2f)\n", proj.start.x, proj.start.y, proj.end.x, proj.end.y);
        debugLog("finish = (%.2f, %.2f), (%.2f, %.2f)\n", finish.start.x, finish.start.y, finish.end.x, finish.end.y);
        debugLog("top = (%.2f, %.2f), (%.2f, %.2f)\n", top.start.x, top.start.y, top.end.x, top.end.y);
        debugLog("bottom = (%.2f, %.2f), (%.2f, %.2f)\n", bottom.start.x, bottom.start.y, bottom.end.x, bottom.end.y);
        */

        vec2 intersect;

        //find nearest intersected plane

        if(proj.intersects(finish, &intersect)) {
            finished=true;
        }
        else if(proj.intersects(bottom, &intersect)) {
            currvel.y = -currvel.y;
            intersect.y -= 1.0f;
        } else if(proj.intersects(top, &intersect)) {
            currvel.y = -currvel.y;
            intersect.y += 1.0f;
        } else {
            break;
        }

        float line_length = glm::length(intersect-p);

        p = intersect;
        points.push_back(intersect);
        line_lengths.push_back(line_length);
        total_distance += line_length;
    }
}

bool ProjectedBall::isFinished() const {
    return has_bounced && distance_travelled >= total_distance;
}

void ProjectedBall::bounce() {
    if(has_bounced) return;

    pos = finish();

    if(!no_bounce) {
        vel.x  = -vel.x;
        dest_x = 0;
    } else {
        dest_x = display.width;
    }

    project();
    has_bounced=true;
}

float ProjectedBall::arrivalTime() {
    return (total_distance-distance_travelled) / (settings.pitch_speed * (float) display.width);
}

float ProjectedBall::getProgress() const {
    return (distance_travelled/total_distance);
}

void ProjectedBall::dontBounce() {
    no_bounce=1;
}

void ProjectedBall::logic(float dt) {
    distance_travelled += dt * settings.pitch_speed * (float) display.width;

    if(distance_travelled >= total_distance) {

        if(!has_bounced) {
            bounce();
        }
        return;
    }

    //number of lines
    int nolines = points.size()-1;

    int pointno = 0;
    float len=0;

    while(pointno<nolines && len+line_lengths[pointno] < distance_travelled) {
        len+=line_lengths[pointno];
        pointno++;
    }

    if(pointno>=nolines) {

        if(!has_bounced) {
            bounce();
            return;
        }

        return;
    }

    vec2 from = points[pointno];
    vec2 to   = points[pointno+1];

    float linepos = (distance_travelled - len)/line_lengths[pointno];

    this->pos = from + ((to-from)*linepos);
}

vec2 ProjectedBall::finish() {
    return points[points.size()-1];
}

float ProjectedBall::getX() {
    return pos.x;
}
