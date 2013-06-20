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

#include "requestball.h"
#include "settings.h"

RequestBall::RequestBall(LogEntry* le, FXFont* font, TextureResource* tex, const vec3& colour, const vec2& pos, const vec2& dest) {
    this->le   = le;
    this->tex  = tex;
    this->font = font;

    vec2 vel = glm::normalize(dest - pos);

    int bytes = le->response_size;
    float size = log((float)bytes) + 1.0f;
    if(size<5.0f) size = 5.0f;

    ProjectedBall::init(pos, vel, colour, (int)dest.x, size);

    start = pos;
    this->dest  = finish();

    if(!le->successful) dontBounce();

    float halfsize = size * 0.5f;
    offset = vec2(halfsize, halfsize);

    char buff[16];
    snprintf(buff, 16, "%s", le->response_code.c_str());
    response_code = std::string(buff);

    response_colour = responseColour();
}

RequestBall::~RequestBall() {
    delete le;
}

vec3 RequestBall::responseColour() {
    return le->response_colour;
}

bool RequestBall::mouseOver(TextArea& textarea, vec2& mouse) {

    //within 3 pixels
    vec2 from_mouse = pos - mouse;

    if( glm::dot(from_mouse, from_mouse) < 36.0f) {

        std::vector<std::string> content;

        content.push_back( std::string( le->path ) );
        content.push_back( " " );

        if(le->vhost.size()>0) content.push_back( std::string("Virtual-Host: ") + le->vhost );

        content.push_back( std::string("Remote-Host:  ") + le->hostname );

        if(le->referrer.size()>0)   content.push_back( std::string("Referrer:     ") + le->referrer );
        if(le->user_agent.size()>0) content.push_back( std::string("User-Agent:   ") + le->user_agent );

        textarea.setText(content);
        textarea.setPos(mouse);
        textarea.setColour(colour);
        return true;
    }

    return false;
}

int RequestBall::logic(float dt) {
    float old_x = pos.x;

    ProjectedBall::logic(dt);

    //returns 1 if just became visible (for score incrementing)
    return (old_x<0.0f && pos.x>=0.0f);
}

void RequestBall::drawGlow() const {
    if(!has_bounced) return;

    float prog = getProgress();

    float glow_radius = size * size * settings.glow_multiplier;

    float alpha = std::min(1.0f, 1.0f-(prog/settings.glow_duration)) * settings.glow_intensity;

    if(alpha <=0.001f) return;
    
    vec3 glow_col = colour * alpha;

    glColor4f(glow_col.x, glow_col.y, glow_col.z, 1.0f);

    glPushMatrix();
        glTranslatef(pos.x, pos.y, 0.0f);

        glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(glow_radius,glow_radius);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(glow_radius,-glow_radius);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-glow_radius,-glow_radius);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-glow_radius,glow_radius);
        glEnd();
    glPopMatrix();
}

void RequestBall::draw() const {

    if(!settings.no_bounce || !has_bounced || no_bounce) {

        vec2 offsetpos = pos - offset;

        glColor4f(colour.x, colour.y, colour.z, 1.0f);

        glPushMatrix();
            glTranslatef(offsetpos.x, offsetpos.y, 0.0f);

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,0.0f);
                glVertex2f(0.0f, 0.0f);

                glTexCoord2f(1.0f,0.0f);
                glVertex2f(size, 0.0f);

                glTexCoord2f(1.0f,1.0f);
                glVertex2f(size, size);

                glTexCoord2f(0.0f,1.0f);
                glVertex2f(0.0f, size);
            glEnd();
        glPopMatrix();
    }
}

void RequestBall::drawResponseCode() const {
    float prog = getProgress();

    float alpha = 1.0f - std::min(1.0f, prog * 2.0f);

    if(alpha<=0.001f) return;
    
    float drift = prog * 100.0f;

    if(!le->successful) drift *= -1.0f;

    vec2 msgpos = (vel * drift) + vec2(dest.x-45.0f, dest.y);
    
    font->setColour(vec4(response_colour.x, response_colour.y, response_colour.z, alpha));
    font->draw(msgpos.x, msgpos.y, response_code.c_str());
}
