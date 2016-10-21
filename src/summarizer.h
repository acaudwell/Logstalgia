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

#include "core/stringhash.h"
#include "core/fxfont.h"
#include "core/regex.h"

#include "textarea.h"

extern const char* summ_wildcard;

class SummNode;
class Summarizer;

class SummRow {
public:
    SummNode* source;

    int words;
    int refs;
    std::string str;
    bool truncated;

    std::vector<std::string> expanded;

    void prependChar(char c);
    void buildSummary(Summarizer* summarizer);
    SummRow();
    SummRow(SummNode* source, bool truncated = false);
};

class SummNode {
public:
    SummNode* parent;

    SummNode();
    SummNode(const std::string& str, size_t offset, SummNode* parent);

    char c;
    int words;
    int refs;

    std::vector<SummNode*> children;
    bool unsummarized;

    void debug(int indent = 0);
    bool addWord(const std::string& str, size_t offset);
    bool removeWord(const std::string& str, size_t offset);

    void expand(Summarizer* summarizer, std::string prefix, std::vector<std::string>& expansion, bool unsummarized_only);

    void summarize(Summarizer* summarizer, std::vector<SummRow>& output, int no_words);
};

class SummItem {
    Summarizer* summarizer;
    vec2 dest;
    vec2 oldpos;

    bool moving;

    float elapsed;
    float eta;
public:
    SummItem(Summarizer* summarizer, SummRow row);

    bool departing;
    bool destroy;

    std::string displaystr;
    int width;

    SummRow row;

    vec4 colour;
    vec2 pos;

    bool isMoving() const;

    void setDest(const vec2& dest);
    void setPos(const vec2& pos);
    void setDeparting(bool departing);
    
    void logic(float dt);
    void draw(float alpha);

    void updateRow(const SummRow& row);
};

class Summarizer {
    std::vector<SummRow> strings;

    std::vector<SummItem> items;
    SummNode root;

    vec3 item_colour;
    bool has_colour;

    float pos_x;
    int max_strings;
    int font_gap;
    FXFont font;

    bool showcount;
    bool right;
    bool changed;

    std::vector<char> delimiters;
    int  abbreviation_depth;

    float incrementf;

    float top_gap, bottom_gap;

    float refresh_delay;
    float refresh_elapsed;

    int screen_percent;

    std::string title;
    std::string prefix_filter;
    Regex matchre;
protected:
    static bool row_sorter(const SummRow &a, const SummRow &b);
    static bool item_sorter(const SummItem &a, const SummItem &b);

public:
    Summarizer(FXFont font, int percent, int abbreviation_depth = 0, float refresh_delay = 2.0f,
               std::string matchstr = ".*", std::string title="");

    void  setPosX(float x);
    float getPosX() const;

    void setSize(int x, float top_gap, float bottom_gap);
    bool isAnimating() const;

    int getScreenPercent();

    bool setPrefixFilterAtPos(const vec2& pos);
    void setPrefixFilter(const std::string& prefix_filter);
    const std::string& getPrefixFilter() const;

    bool mouseOver(const vec2 &pos) const;
    bool getInfoAtPos(TextArea& textarea, const vec2& pos);

    bool hasColour() const;
    void setColour(const vec3& col);
    const vec3& getColour() const;

    void setShowCount(bool showcount);
    bool showCount() const;

    void setAbbreviationDepth(int abbreviation_depth);
    int getAbbreviationDepth() const;

    FXFont& getFont();

    bool supportedString(const std::string& str);

    void removeString(const std::string& str);
    void addString(const std::string& str);

    void addDelimiter(char c);
    bool isDelimiter(char c) const;

    const std::string& getBestMatchStr(const std::string& str) const;
    int         getBestMatchIndex(const std::string& str) const;
    float       getPosY(const std::string& str) const;
    float       getMiddlePosY(const std::string& str) const;

    float calcPosY(int i) const;

    void summarize();

    void recalc_display();
    void logic(float dt);
    void draw(float dt, float alpha);
};

#endif
