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
#include "requestball.h"
#include "settings.h"

#include "core/stringhash.h"

Paddle::Paddle(vec2 pos, vec4 colour, std::string token, FXFont font) {
    this->token = token;

// TODO: fix colouring
//    this->token_colour = token.size() > 0 ? colourHash2(token) : vec3(0.5,0.5,0.5);
    this->token_colour = token.size() > 0 ? colourHash(token) : vec3(0.5,0.5,0.5);

    this->pos = pos;
    this->lastcol = colour;
    this->default_colour = colour;
    this->colour  = lastcol;
    this->width = 10;
    this->height = 50;
    this->target = 0;

    font.alignTop(true);
    font.alignRight(true);
    font.dropShadow(true);

    this->font = font;

    dest_y = -1;
}

Paddle::~Paddle() {
}

void Paddle::moveTo(int y, float eta, vec4 nextcol) {
    this->start_y = (int) this->pos.y;
    this->dest_y = y;
    this->dest_eta = eta;
    this->dest_elapsed = 0.0f;
    this->nextcol = nextcol;

    //debugLog("move to %d over %.2f\n", dest_y, dest_eta);
}

bool Paddle::visible() {
    return colour.w > 0.01;
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

RequestBall* Paddle::getTarget() {
    return target;
}

void Paddle::setTarget(RequestBall* target) {
    this->target = target;

    if(target==0) {
        moveTo(display.height/2, 4, default_colour);
        return;
    }

    vec2 dest = target->finish();
    vec4 col  = (settings.paddle_mode == PADDLE_VHOST || settings.paddle_mode == PADDLE_PID)  ?
        vec4(token_colour,1.0) : vec4(target->colour, 1.0f);

    moveTo((int)dest.y, target->arrivalTime(), col);
}

bool Paddle::mouseOver(TextArea& textarea, vec2& mouse) {

    if(pos.x <= mouse.x && pos.x + width >= mouse.x && abs(pos.y - mouse.y) < height/2) {

        std::vector<std::string> content;

        content.push_back( token );

        textarea.setText(content);
        textarea.setPos(mouse);
        textarea.setColour(vec3(colour));

        return true;
    }

    return false;
}

void Paddle::logic(float dt) {

    if(dest_y != -1) {
        float remaining = dest_eta - dest_elapsed;

        if(remaining<0.0f) {
            //debugLog("paddle end point reached\n");
            pos.y = dest_y;
            dest_y = -1;
            target = 0;
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

void Paddle::drawToken() {
    font.setColour(colour);
    font.draw(pos.x-10, pos.y - (font.getMaxHeight()/2), token);
}

void Paddle::drawShadow() {

    vec2 spos = vec2(pos.x + 1.0f, pos.y + 1.0f);

    glColor4f(0.0, 0.0, 0.0, 0.7 * colour.w);
    glBegin(GL_QUADS);
        glVertex2f(spos.x,spos.y-(height/2));
        glVertex2f(spos.x,spos.y+(height/2));
        glVertex2f(spos.x+width,spos.y+(height/2));
        glVertex2f(spos.x+width,spos.y-(height/2));
    glEnd();
}

void Paddle::draw() {

    glColor4fv(glm::value_ptr(colour));
    glBegin(GL_QUADS);
        glVertex2f(pos.x,pos.y-(height/2));
        glVertex2f(pos.x,pos.y+(height/2));
        glVertex2f(pos.x+width,pos.y+(height/2));
        glVertex2f(pos.x+width,pos.y-(height/2));
    glEnd();
}
