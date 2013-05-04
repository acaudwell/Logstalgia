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
#ifndef TEXTAREA_H
#define TEXTAREA_H

#include <vector>
#include <string>

#include "core/display.h"
#include "core/vectors.h"
#include "core/fxfont.h"

class TextArea {

    vec3 colour;
    vec2 corner;
    std::vector<std::string> content;
    FXFont font;
    int rectwidth;
    int rectheight;
    bool visible;
public:
    TextArea();
    TextArea(FXFont font);
    TextArea(std::vector<std::string>& content, FXFont font, vec3 colour);

    void hide();
    void setText(std::vector<std::string>& content);
    void setPos(vec2 pos);
    void setColour(vec3 colour);
    void draw();

};

#endif
