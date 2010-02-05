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
 
#include "paddle.h"

bool gPaddle=true;

Paddle::Paddle(vec2f pos, vec3f colour) {
    this->pos = pos;
    this->lastcol = vec4f(colour, 1.0f);
    this->colour  = lastcol;
    this->width = 10;
    this->height = 50;

    dest_y = -1;
}

Paddle::~Paddle() {
}

void Paddle::moveTo(int y, float eta, vec3f nextcol) {
    this->start_y = (int) this->pos.y;
    this->dest_y = y;
    this->dest_eta = eta;
    this->dest_elapsed = 0.0f;
    this->nextcol = vec4f(nextcol, 1.0f);

    debugLog("move to %d over %.2f\n", dest_y, dest_eta);
}

bool Paddle::moving() {
    return dest_y != -1;
}

float Paddle::getY() {
    return pos.y;
}

float Paddle::getX() {
    return pos.x;
}

void Paddle::logic(float dt) {

    if(dest_y != -1) {
        float remaining = dest_eta - dest_elapsed;

        if(remaining<0.0f) {
            debugLog("paddle end point reached\n");
            pos.y = dest_y;
            dest_y = -1;
            colour = nextcol;
            lastcol = colour;
        } else {
            float alpha = remaining/dest_eta;
            pos.y = start_y + ((dest_y-start_y)*(1.0f - alpha));
            colour = lastcol * alpha + nextcol * (1.0f - alpha);
        }

        dest_elapsed += dt;
    }
}

void Paddle::draw(float dt) {
    if(!gPaddle) return;

    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    vec2f spos = vec2f(pos.x + 1.0f, pos.y + 1.0f);

    glColor4f(0.0, 0.0, 0.0, 0.7);
    glBegin(GL_QUADS);
        glVertex2f(spos.x,spos.y-(height/2));
        glVertex2f(spos.x,spos.y+(height/2));
        glVertex2f(spos.x+width,spos.y+(height/2));
        glVertex2f(spos.x+width,spos.y-(height/2));
    glEnd();

    glColor4fv(colour);
    glBegin(GL_QUADS);
        glVertex2f(pos.x,pos.y-(height/2));
        glVertex2f(pos.x,pos.y+(height/2));
        glVertex2f(pos.x+width,pos.y+(height/2));
        glVertex2f(pos.x+width,pos.y-(height/2));
    glEnd();
}
