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

//Line
Line::Line(vec2f start, vec2f end) {
	this->start = start;
	this->end   = end;
}

Line::~Line() {
}

// check if line intersects the plane of another line

bool Line::intersects(Line& l, vec2f *p) {
    vec2f a = end   - start;
    vec2f b = l.end - l.start;

    float d = a.x*b.y - a.y * b.x;

    //parallel test
    if (!d) return false;

    vec2f w = start - l.start;

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

ProjectedBall::ProjectedBall(const vec2f& pos, const vec2f& vel, const vec3f& colour, int dest_x, float eta, float size, float speed) {
    init(pos, vel, colour, dest_x, eta, size, speed);
}

void ProjectedBall::init(const vec2f& pos, const vec2f& vel, const vec3f& colour, int dest_x, float eta, float size, float speed) {
    this->pos = pos;
    this->vel = vel;
    this->colour = colour;
    this->speed = speed;
    this->size  = size;

    this->dest_x = dest_x;
    this->start_x = (int) pos.x;
    this->eta    = eta;
    this->has_bounced=0;
    no_bounce = 0;

    project();
}


ProjectedBall::~ProjectedBall() {
}

/*project path of the ball
  as a series of line segments
*/

void ProjectedBall::project() {
    elapsed = 0.0f;
    progress = 0.0f;
    points.clear();
    vec2f p = pos;
    points.push_back(p);

    //project current position and direction until it hits a wall or crosses dest_x
    Line finish(vec2f(dest_x, 0), vec2f(dest_x, display.height));
    Line top(vec2f(0,0), vec2f(display.width, 0));
    Line bottom(vec2f(0,display.height), vec2f(display.width, display.height));

    //project far enough to cross a side of the screen
    float inc = display.width*2.0f;

    bool finished=false;
    vec2f currvel = vel;
    total_length=0;
    line_lengths.clear();

    while(!finished) {
        Line proj(p, p + currvel*inc);

        /*
        debugLog("proj = (%.2f, %.2f), (%.2f, %.2f)\n", proj.start.x, proj.start.y, proj.end.x, proj.end.y);
        debugLog("finish = (%.2f, %.2f), (%.2f, %.2f)\n", finish.start.x, finish.start.y, finish.end.x, finish.end.y);
        debugLog("top = (%.2f, %.2f), (%.2f, %.2f)\n", top.start.x, top.start.y, top.end.x, top.end.y);
        debugLog("bottom = (%.2f, %.2f), (%.2f, %.2f)\n", bottom.start.x, bottom.start.y, bottom.end.x, bottom.end.y);
        */

        vec2f intersect;

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

        float length = (intersect-p).length();

        p = intersect;
        points.push_back(intersect);
        line_lengths.push_back(length);
        total_length +=length;
    }
}

bool ProjectedBall::isFinished() const {
    return has_bounced && elapsed>=eta;
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

bool ProjectedBall::arrived() const {
    return elapsed>=eta;
}

void ProjectedBall::setElapsed(float e) {
    elapsed =e;
}

float ProjectedBall::arrivalTime() {
    return (eta-elapsed)/speed;
}

float ProjectedBall::getProgress() const {
    return progress;
}

void ProjectedBall::dontBounce() {
    no_bounce=1;
}

void ProjectedBall::logic(float dt) {
    elapsed += (dt);// * speed);
    progress = elapsed / eta;

    if(progress>1.0f) {
        if(!has_bounced) {
            bounce();
            return;
        }

        return;
    }

    //number of lines
    int nolines = points.size()-1;

    //progress
    float currposf = progress * total_length;

    int pointno = 0;
    float len=0;

    while(pointno<nolines && len+line_lengths[pointno]<currposf) {
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

    vec2f from = points[pointno];
    vec2f to   = points[pointno+1];

    float linepos = (currposf - len)/line_lengths[pointno];

    this->pos = from + ((to-from)*linepos);
}

vec2f ProjectedBall::finish() {
    return points[points.size()-1];
}

float ProjectedBall::getX() {
    return pos.x;
}
