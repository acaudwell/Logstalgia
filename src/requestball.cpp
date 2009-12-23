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

int gHighscore = 0;

bool gBounce=true;
bool gResponseCode=true;

RequestBall::RequestBall(LogEntry& le, FXFont font, TextureResource* tex, vec3f colour, vec2f pos, vec2f dest, float speed) {
    this->le  = le;
    this->font = font;
    this->tex = tex;

    font.dropShadow(true);

    vec2f vel = dest - pos;
    vel.normalize();

    int bytes = le.bytesCount();
    float size = log((float)bytes) + 1.0f;
    if(size<5.0f) size = 5.0f;

    float eta = 5;

    ProjectedBall::init(pos, vel, colour, (int)dest.x, eta, size, speed);

    this->speed = speed;

    start = pos;
    this->dest  = finish();

    if(!le.successful()) dontBounce();

    char buff[16];
    snprintf(buff, 16, "%d", le.responseCode());
    response_code = std::string(buff);

    response_colour = responseColour();
}

RequestBall::~RequestBall() {
}

vec3f RequestBall::responseColour() {
    int code = le.responseCode();

    if(code<200) {
        return vec3f(0.0f, 1.0f, 0.5f);
    }

    if(code>= 200 && code < 300) {
        return vec3f(1.0f, 1.0f, 0.0f);
    }

    if(code>= 300 && code < 400) {
        return vec3f(1.0f, 0.5f, 0.0f);
    }

    return vec3f(1.0f, 0.0f, 0.0f);
}

bool RequestBall::mouseOver(TextArea& textarea, vec2f& mouse) {
    //within 3 pixels
    if((pos - mouse).length2()<36.0f) {

        std::vector<std::string> content;
        content.push_back( std::string("addr:  ") + le.getHostname() );
        content.push_back( std::string("url:     ") + le.requestURL() );
        content.push_back( std::string("ref:     ") + le.referrerURL() );
        content.push_back( std::string("agent: ") + le.userAgent() );

        textarea.setText(content);
        textarea.setPos(pos);
        textarea.setColour(colour);
        return true;
    }

    return false;
}

//increment score by response code
void RequestBall::logic(float dt) {
    ProjectedBall::logic(dt);
}

void RequestBall::draw(float dt) {
//    glDisable(GL_TEXTURE_2D);

    bool hasBounced = bounced();

    glEnable(GL_TEXTURE_2D);

    if(gBounce || !hasBounced || no_bounce) {
        glBindTexture(GL_TEXTURE_2D, tex->textureid);
//       glEnable(GL_BLEND);
//       glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float halfsize = size * 0.5f;
        vec2f offsetpos = pos - vec2f(halfsize, halfsize);

        glColor4f(0.0f, 0.0f, 0.0f, 0.9f);

/*
    drop shadow

        glPushMatrix();
            glTranslatef(2.0f, 2.0f, 0.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0f,0.0f);
                glVertex2f(offsetpos.x, offsetpos.y);

                glTexCoord2f(1.0f,0.0f);
                glVertex2f(offsetpos.x+size, offsetpos.y);

                glTexCoord2f(1.0f,1.0f);
                glVertex2f(offsetpos.x+size, offsetpos.y+size);

                glTexCoord2f(0.0f,1.0f);
                glVertex2f(offsetpos.x, offsetpos.y+size);
            glEnd();
        glPopMatrix();
*/

        glColor4f(colour.x, colour.y, colour.z, 1.0f);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);
            glVertex2f(offsetpos.x, offsetpos.y);

            glTexCoord2f(1.0f,0.0f);
            glVertex2f(offsetpos.x+size, offsetpos.y);

            glTexCoord2f(1.0f,1.0f);
            glVertex2f(offsetpos.x+size, offsetpos.y+size);

            glTexCoord2f(0.0f,1.0f);
            glVertex2f(offsetpos.x, offsetpos.y+size);
        glEnd();

/*
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINE_STRIP);
            for(int i=0;i<points.size();i++) {
                glVertex2fv(points[i]);
            }
        glEnd();
*/
    }

    glEnable(GL_TEXTURE_2D);
//   	glEnable(GL_BLEND);
//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec3f hostcol = colour;
    vec3f pagecol = pagecolour;

    if(hasBounced && gResponseCode) {
        float prog = progress();
        float drift = prog * 100.0f;

        if(!le.successful()) drift *= -1.0f;
        vec2f msgpos = (vel * drift) + vec2f(dest.x-45.0f, dest.y);

        glColor4f(response_colour.x, response_colour.y, response_colour.z, 1.0f - std::min(1.0f, prog * 2.0f) );
        font.draw((int)msgpos.x, (int)msgpos.y, response_code.c_str());
    }
}
