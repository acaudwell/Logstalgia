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

#ifndef SUMMARIZER_H
#define SUMMARIZER_H

#include <list>

#include "core/stringhash.h"
#include "core/fxfont.h"
#include "core/regex.h"

#include "textarea.h"

extern const char* summ_wildcard;

class SummNode;

class SummUnit {
public:
    SummNode* source;

    int words;
    int refs;
    std::string str;
    bool truncated;
    bool exceptions;

    std::vector<std::string> expanded;

    void prependChar(char c);
    void buildSummary();
    SummUnit();
    SummUnit(SummNode* source, bool truncated = false, bool exceptions = false);
};

class SummNode {
public:
    SummNode* parent;

    SummNode();
    SummNode(const std::string& str, SummNode* parent);
    char c;
    int words;
    int refs;
    bool is_root;
    std::vector<SummNode*> children;
    std::vector<bool> exception;

    void debug(int indent = 0);
    bool addWord(const std::string& str);
    bool removeWord(const std::string& str);

    void expand(std::string prefix, std::vector<std::string>& expansion, bool exceptions);

    int summarize(std::vector<SummUnit>& strvec, int no_words);
};

class SummItem {
    vec2f dest;
    vec2f oldpos;

    bool moving;

    float elapsed;
    float eta;
    float target_x;

    vec3f* icol;
    bool showcount;
    FXFont font;
public:
    bool departing;
    bool destroy;

    std::string displaystr;
    int width;

    SummUnit unit;

    vec4f colour;
    vec2f pos;

    void setDest(vec2f dest, bool depart = false);
    void logic(float dt);
    void draw(float alpha);

    void updateUnit(SummUnit& unit);
    SummItem(SummUnit unit, vec2f pos, vec2f dest, float target_x, vec3f* icol, FXFont font, bool showcount);
};

class Summarizer {
    std::vector<SummUnit> strings;

    std::list<SummItem> items;
    SummNode root;

    vec3f* item_colour;

    float pos_x;
    int max_strings;
    int font_gap;
    FXFont font;

    bool showcount;
    bool right;
    bool mouseover;
    bool changed;
    
    float incrementf;

    float top_gap, bottom_gap;

    float refresh_delay;
    float refresh_elapsed;

    std::string title;
    Regex matchre;

public:
    Summarizer(FXFont font, float x, float top_gap = 0.0f, float bottom_gap = 0.0f, float refresh_delay = 2.0f,
               std::string matchstr = ".*", std::string title="");
    ~Summarizer();


    bool mouseOver(TextArea& textarea, vec2f mouse);
    void mouseOut();

    bool isColoured();
    void showCount(bool showcount);
    void setColour(vec3f col);
    vec3f getColour();

    bool supportedString(std::string& str);

    void removeString(const std::string& str);
    void addString(const std::string& str);

    const std::string& getBestMatchStr(const std::string& str);
    int         getBestMatch(const  std::string& str);
    float       getPosY(const std::string& str);
    float       getMiddlePosY(const std::string& str);

    float calcPosY(int i);

    void summarize();

    void recalc_display();
    void logic(float dt);
    void draw(float dt, float alpha);

};

#endif
