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

#include "summarizer.h"

#include <algorithm>

/*

Method:
if leaf:
    - return string
if has children:
    - look at each childs 'word' count and allocate a % of the no_strings to each
      child where the % is  the total word count of the children / the individual count
      of each node (rounding downwards - ie 0.9 -> 0)

      call each child and return the total set of strings from the children.

*/

//SummUnit
SummUnit::SummUnit() {
    this->words=0;
    this->refs=0;
    this->truncated=false;
    this->exceptions=false;
}

SummUnit::SummUnit(SummNode* source, bool truncated, bool exceptions) {
    this->source    = source;
    this->words     = source->words;
    this->refs      = source->refs;
    this->truncated = truncated;
    this->exceptions= exceptions;

    if(source->parent!=0) prependChar(source->c);
}

void SummUnit::buildSummary() {
    expanded.clear();
    source->expand(str, expanded, exceptions);
}

void SummUnit::prependChar(char c) {
    str.insert(0,1,c);
}

//SummNode

const char* summ_wildcard = "*";

SummNode::SummNode() {
    c = '*';
    words=0;
    parent=0;
}

SummNode::SummNode(const std::string& str, size_t offset, SummNode* parent) {
    c = str[offset];
    words=0;
    refs=0;
    this->parent=parent;

    //if leaf
    if(!addWord(str, ++offset)) {
         words=1;
    }
}

bool SummNode::removeWord(const std::string& str, size_t offset) {

    refs--;

    size_t str_size = str.size() - offset;

    if(!str_size) return false;

    words--;

    size_t no_children = children.size();

    bool removed = false;

    for(size_t i=0;i<no_children;i++) {
        if(children[i]->c == str[offset]) {
            removed = children[i]->removeWord(str,++offset);

            if(children[i]->refs == 0) {
                std::vector<SummNode*>::iterator it = children.begin()+i;
                delete *it;
                children.erase(it);
            }

            break;
        }
    }

    return removed;
}

void SummNode::debug(int indent) {

    std::string indentation;
    if(indent>0) indentation.append(indent, ' ');

    debugLog("%snode c=%c refs=%d words=%d", indentation.c_str(), c, refs, words);
    indent++;

    for(size_t i=0;i<children.size();i++) {
        SummNode* child = children[i];
        child->debug(indent);
    }
}

bool SummNode::addWord(const std::string& str, size_t offset) {

    refs++;

    size_t str_size = str.size() - offset;

    if(!str_size) return false;

    words++;

    size_t no_children = children.size();

    for(size_t i=0;i<no_children;i++) {
        if(children[i]->c == str[offset]) {
            return children[i]->addWord(str, ++offset);
        }
    }

    children.push_back(new SummNode(str, offset, this));

    return true;
}

std::string format_node(std::string str, int refs) {
    char buff[256];
    snprintf(buff, 256, "%03d %s", refs, str.c_str());

    return std::string(buff);
}

void SummNode::expand(std::string prefix, std::vector<std::string>& vec, bool exceptions) {
    size_t no_child = children.size();

    if(no_child==0) {
        vec.push_back(format_node(prefix, refs));
        return;
    }

    //find top-but-not-root node, expand root node
    std::vector<SummUnit>::iterator it;

    for(size_t i=0;i<no_child;i++) {
        if(exceptions && !exception[i]) continue;

        std::vector<SummUnit> strvec;
        children[i]->summarize(strvec, 100);

        for(it=strvec.begin(); it!=strvec.end(); it++) {
            vec.push_back(format_node(prefix+(*it).str, children[i]->refs));
        }
    }
}

int SummNode::summarize(std::vector<SummUnit>& strvec, int no_words) {

    // if no children, just append this node
    if(children.size()==0 && parent!=0) {
        strvec.push_back(SummUnit(this));
        return 1;
    }

    int total_count=0;
    //get total number of words

    size_t no_child = children.size();

    //figure out the total number of words 'below' this node
    int total_child_words = 0;
    for(size_t i=0;i<no_child;i++) {
        total_child_words += children[i]->words;
    }

    //distribute slots to children
    int extra_slots  = no_words;
    int un_covered=0;

    for(size_t i=0;i<no_child;i++) {
        float percent   = (float) children[i]->words / total_child_words;
        int child_share = (int)(percent * no_words);
        extra_slots -= child_share;

        if(child_share==0) un_covered++;
    }

    //if any child will be left out need a spare catch all slot
    if(un_covered>extra_slots) {
        extra_slots--;
    }

    exception.clear();
    un_covered=0;
    int last_uncovered=-1;

    //summarize children,
    for(size_t i=0;i<no_child;i++) {
        float percent   = (float) children[i]->words / total_child_words;
        int child_share = (int)(percent * no_words);

        //give any left over slots to largest child
        if(extra_slots>0 && child_share==0) {

            if(i==no_child-1) {
                child_share += extra_slots;
                extra_slots = 0;
            } else {
                child_share += 1;
                extra_slots -= 1;
            }
        }

        if(child_share<=0) {
            un_covered++;
            exception.push_back(true);
            last_uncovered=i;
            continue;
        }

        exception.push_back(false);

        int currsize = strvec.size();
        int count = children[i]->summarize(strvec, child_share);
        total_count+=count;
        size_t newsize = (size_t) (count + currsize);

        for(size_t j=currsize;j<newsize;j++) {
            if(parent!=0) strvec[j].prependChar(c);
        }
    }

    if(un_covered>0) {

        if(un_covered==1) {
            int currsize = strvec.size();
            int count = children[last_uncovered]->summarize(strvec, 1);
            size_t newsize = (size_t) (count + currsize);

            total_count += count;

            for(size_t j=currsize;j<newsize;j++) {
                if(parent!=0) strvec[j].prependChar(c);
            }

            return total_count;
        }

        strvec.push_back(SummUnit(this, true, true));
        return total_count+1;
    }

    return total_count;
}


// SummItem
void SummItem::updateUnit(SummUnit& unit) {

    this->unit = unit;

    vec3 col = icol!=0 ? *icol : colourHash(unit.str);
    this->colour = vec4(col, 1.0f);

    char buff[1024];

    if(unit.truncated) {
        if(showcount) {
            snprintf(buff, 1024, "%03d %s (%d)", unit.refs, unit.str.c_str(), (int) unit.expanded.size());
        } else {
            snprintf(buff, 1024, "%s (%d)", unit.str.c_str(), (int) unit.expanded.size());
        }
    } else {
        if(showcount) {
        snprintf(buff, 1024, "%03d %s", unit.refs, unit.str.c_str());
        } else {
        snprintf(buff, 1024, "%s", unit.str.c_str());
        }
    }

    this->displaystr = std::string(buff);
    this->width = font.getWidth(displaystr);

}

SummItem::SummItem(SummUnit unit, float target_x, vec3* icol, FXFont font, bool showcount) {
    this->pos  = vec2(-1.0,-1.0);
    this->dest = vec2(-1.0,-1.0);
    this->target_x = target_x;
    this->icol = icol;
    this->font = font;
    this->showcount=showcount;

    updateUnit(unit);

    destroy=false;
    moving=false;
    departing=false;

    setDest(dest);
}

void SummItem::setDest(const vec2& dest) {
    vec2 dist = this->dest - dest;

    if(moving && glm::dot(dist, dist) < 1.0f) return;

    this->oldpos = pos;
    this->dest   = dest;
    this->elapsed = 0;
    this->eta = 1.0f;

    destroy   = false;
    moving    = true;
}

void SummItem::setDeparting(bool departing) {
    this->departing = departing;
    if(!departing) destroy = false;
}

void SummItem::setPos(const vec2& pos) {
    this->pos = pos;
}

void SummItem::logic(float dt) {
    if(!moving) return;

    float remaining = eta - elapsed;

    if(remaining>0.0f) {
        float dist_x = target_x - pos.x;
        if(dist_x<0.0f) dist_x = -dist_x;

        float alpha = 0.0f;
        if(dist_x<200.0f) {
            alpha = 1.0f - (dist_x/200.0f);
        }

        colour.w = alpha;

        pos = oldpos + (dest-oldpos)*(1.0f - (remaining/eta));
        elapsed+=dt;
    } else {
        pos = dest;
        if(departing) {
            destroy = true;
        }
        departing = false;
        moving    = false;
        colour.w=1.0f;
    }
}

void SummItem::draw(float alpha) {
    font.setColour(vec4(colour.x, colour.y, colour.z, colour.w * alpha));
    font.draw((int)pos.x, (int)pos.y, displaystr.c_str());
}

// Summarizer

Summarizer::Summarizer(FXFont font, int screen_percent, float refresh_delay, std::string matchstr, std::string title)
    : matchre(matchstr) {
    pos_x = top_gap = bottom_gap = 0.0f;

    this->screen_percent = screen_percent;
    this->title      = title;
    this->font       = font;

    this->refresh_delay   = refresh_delay;
    this->refresh_elapsed = refresh_delay;

    this->item_colour=0;
    this->showcount=false;

    changed = false;

    incrementf   =0;
    root = SummNode();

    mouseover=false;
}

int Summarizer::getScreenPercent() {
    return screen_percent;
}

void Summarizer::setSize(int x, float top_gap, float bottom_gap) {
    this->pos_x      = x;
    this->top_gap    = top_gap;
    this->bottom_gap = bottom_gap;

    // TODO: set 'right' explicitly?
    right = (pos_x > (display.width/2)) ? true : false;

    font_gap = font.getMaxHeight() + 4;

    max_strings = (int) ((display.height-top_gap-bottom_gap)/font_gap);

    if(!title.empty()) {
        this->top_gap+= font_gap;
    }

    changed = true;
    refresh_elapsed = refresh_delay;
    items.clear();
}

void Summarizer::mouseOut() {
    mouseover=false;
}

bool Summarizer::mouseOver(TextArea& textarea, vec2 mouse) {
    mouseover=false;

    if(right && mouse.x < pos_x) return false;
    if(mouse.y < top_gap || mouse.y > (display.height-bottom_gap)) return false;
    if(items.empty()) return false;

    float y = mouse.y;

    for(SummItem& item : items) {
        if(item.departing) continue;

        if(item.pos.y<=y && (item.pos.y+font.getMaxHeight()+4) > y) {
            if(mouse.x< item.pos.x || mouse.x > item.pos.x + item.width) continue;

            std::vector<std::string> content;

            textarea.setText(item.unit.expanded);
            textarea.setColour(vec3(item.colour));
            textarea.setPos(mouse);
            mouseover=true;
            return true;
        }
    }

    return false;
}

void Summarizer::setColour(vec3 col) {
    this->item_colour = new vec3(col);
}

bool Summarizer::isColoured() {
    return (item_colour!=0);
}

vec3 Summarizer::getColour() {
    return *item_colour;
}

Summarizer::~Summarizer() {
    if(item_colour!=0) delete item_colour;
}

bool Summarizer::supportedString(const std::string& str) {
    return matchre.match(str);
}

// string sort with numbers sorted last (reasoning: text is more likely to be interesting)
bool _unit_sorter(const SummUnit& a, const SummUnit& b) {

    int ai = atoi(a.str.c_str());
    int bi = atoi(b.str.c_str());
    
    // if only one of a or b is numeric, non numeric value wins
    if((ai==0) != (bi==0)) {
        return ai==0;
    }

    return a.str.compare(b.str) < 0;
}

bool _item_sorter(const SummItem& a, const SummItem& b) {   
    return _unit_sorter(a.unit,b.unit);
}

void Summarizer::summarize() {
    if(!changed) return;

    changed = false;

    strings.clear();
    root.summarize(strings,max_strings);

    size_t nostrs = strings.size();

    for(size_t i=0;i<nostrs;i++) {
        strings[i].buildSummary();
    }

    std::sort(strings.begin(), strings.end(), _unit_sorter);
    
    if(nostrs>1) {
        incrementf  = (((float)display.height-font_gap-top_gap-bottom_gap)/(nostrs-1));
    } else {
        incrementf =0;
    }
}

void Summarizer::recalc_display() {

    size_t nostrs = strings.size();

    std::vector<bool> strfound;
    strfound.resize(nostrs, false);

    size_t found_count = 0;
    
    //update summItems
    for(SummItem& item : items) {

        int match = -1;

        for(size_t j=0;j<nostrs;j++) {
            SummUnit summstr = strings[j];

            if(summstr.str.compare(item.unit.str) == 0) {
                item.updateUnit(summstr);

                match = (int)j;

                strfound[j]= true;
                found_count++;
                
                break;
            }
        }

        item.setDeparting(match == -1 ? true : false);
    }

    //add items for strings not found
    for(size_t i=0;i<nostrs;i++) {
        if(strfound[i]) continue;

        items.push_back(SummItem(strings[i], pos_x, item_colour, font, showcount));
        
        //debugLog("added item for unit %s %d", strings[i].str.c_str(), items[items.size()-1].destroy);
    }
    
    // sort items alphabetically
    std::sort(items.begin(), items.end(), _item_sorter);

    // set y positions

    int y_index = 0;
        
    for(SummItem& item : items) {
        
        float Y = calcPosY(y_index);

        if(!item.departing) y_index++;

        float startX = right ? display.width + 100 : -200;

        // initialize new item position
        if(item.pos.y<0.0) {
            item.setPos(vec2(startX, Y));
        }
        
        if(item.departing) {
            // return to start
            item.setDest(vec2(startX, Y));
        } else {
            item.setDest(vec2(pos_x, Y));
        }
        
    }
}

void Summarizer::removeString(const std::string& str) {
    root.removeWord(str,0);
    changed = true;
}

float Summarizer::calcPosY(int i) const {
    return top_gap + (incrementf * i) ;
}

int Summarizer::getBestMatchIndex(const std::string& input) const {

    int best_diff = -1;
    int best      = -1;
    int best_size = -1;

    size_t nostrs = strings.size();

    for(size_t i = 0;i < nostrs; i++) {

        size_t strn_size  = strings[i].str.size();
        size_t input_size = input.size();

        int size_diff = strn_size - input_size;

        size_t min_size   = size_diff == 0 ? input_size : std::min ( strn_size, input_size );

        int min_common_diff = abs( strings[i].str.compare(0, min_size, input, 0, min_size) );

        //found
        if(size_diff == 0 && min_common_diff == 0) {
            best = i;
            break;
        }

        if(    best_diff == -1
            || min_common_diff < best_diff
            || (min_common_diff == best_diff //remove this?
               && min_size > best_size
               && strn_size < input_size)) {

            best = i;

            best_diff = min_common_diff;
            best_size = min_size;
        }
    }

    return best;
}

const std::string& Summarizer::getBestMatchStr(const std::string& str) const {
    int pos = getBestMatchIndex(str);

    assert(pos !=- 1);

    return strings[pos].str;
}


float Summarizer::getMiddlePosY(const std::string& str) const {
    return getPosY(str) + (font.getMaxHeight()) / 2;
}

float Summarizer::getPosY(const std::string& str) const {

    int best = getBestMatchIndex(str);

    if(best!=-1) {
        return calcPosY(best);
    }

    return calcPosY(0);
}

void Summarizer::showCount(bool showcount) {
    this->showcount = showcount;
}

void Summarizer::addString(const std::string& str) {
    root.addWord(str,0);
    changed = true;
}

void Summarizer::logic(float dt) {

    if(changed) summarize();

    refresh_elapsed+=dt;
    if(refresh_elapsed>=refresh_delay) {
        recalc_display();
        refresh_elapsed=0;
    }

    //move items
    for(auto it = items.begin();it != items.end();) {
        (*it).logic(dt);

        if((*it).destroy) {
            it = items.erase(it);
        } else {
            it++;
        }
    }
}

void Summarizer::draw(float dt, float alpha) {
   	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

    if(title.size()) {
        font.setColour(vec4(1.0f, 1.0f, 1.0f, alpha));
        font.draw((int)pos_x, (int)(top_gap - font_gap), title.c_str());
    }

    for(SummItem& item : items) {
        item.draw(alpha);
    }

}
