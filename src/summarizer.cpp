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

SummNode::SummNode(std::string& str, SummNode* parent) {
    c = str[0];
    words=0;
    refs=0;
    this->parent=parent;

    //if leaf
    if(!addWord(str)) {
         words=1;
    }
}

bool SummNode::removeWord(std::string& str) {

    bool removed=false;

    if(str.size()>1) {
        std::string substr = parent==0 ? str : std::string(str, 1, str.size()-1);

        for(size_t i=0;i<children.size();i++) {
            if(children[i]->c == substr[0]) {
                removed = children[i]->removeWord(substr);

                if(children[i]->refs==0) {
                    //debugLog("removing child at %d with char %c\n", substr[0]);
                    std::vector<SummNode*>::iterator it = children.begin()+i;
                    delete *it;
                    children.erase(it);
                    removed=true;
                }

                break;
            }
        }
    }

    refs--;

    if(removed) {
        words--;
    }

    return removed;
}

void SummNode::debug(int indent) {
    for(int i=0;i<indent;i++)
        debugLog(" ");

    debugLog("node c=%c refs=%d words=%d\n", c, refs, words);
    indent++;

    for(size_t i=0;i<children.size();i++) {
        SummNode* child = children[i];
        child->debug(indent);
    }
}

bool SummNode::addWord(std::string& str) {

    //debugLog("adding %s to node '%c'\n", str.c_str(), c);
    bool added=false;

    if(str.size()>1) {
        std::string substr = parent==0 ? str : std::string(str, 1, str.size()-1);

        SummNode* child = 0;
        for(size_t i=0;i<children.size();i++) {
            if(children[i]->c ==substr[0]) {
                child = children[i];
                break;
            }
        }

        if(child!=0) {
            added = child->addWord(substr);
        } else {
            children.push_back(new SummNode(substr, this));
            added=true;
        }
    }

    refs++;

    if(added) {
        words++;
    }

    //debugLog("total words for %c = %d\n", c, words);
    return added;
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

    vec3f col = icol!=0 ? *icol : colourHash(unit.str);
    this->colour = vec4f(col, 1.0f);

    char buff[1024];

    if(unit.truncated) {
        if(showcount) {
            snprintf(buff, 1024, "%03d %s (%d)", unit.refs, unit.str.c_str(), unit.expanded.size());
        } else {
            snprintf(buff, 1024, "%s (%d)", unit.str.c_str(), unit.expanded.size());
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

SummItem::SummItem(SummUnit unit, vec2f pos, vec2f dest, float target_x, vec3f* icol, FXFont font, bool showcount) {
    this->pos  = pos;
    this->target_x = target_x;
    this->icol = icol;
    this->font = font;
    this->showcount=showcount;

    updateUnit(unit);

    destroy=false;
    moving=false;

    setDest(dest);
}

void SummItem::setDest(vec2f dest, bool depart) {
    if(moving && (this->dest - dest).length2()<1.0f) return;

    this->oldpos = pos;
    this->dest   = dest;
    this->elapsed = 0;
    this->eta = 1.0f;

    destroy   = false;
    moving    = true;
    departing = depart;
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

void SummItem::draw() {
    glColor4fv(colour);
    font.draw((int)pos.x, (int)pos.y, displaystr.c_str());
}

// Summarizer

Summarizer::Summarizer(FXFont font, float x, float top_gap, float bottom_gap, float refresh_delay, std::string matchstr, std::string title)
    : matchre(matchstr)
 {
    this->pos_x      = x;
    this->top_gap    = top_gap;
    this->bottom_gap = bottom_gap;
    this->title      = title;
    this->font       = font;

    this->refresh_delay   = refresh_delay;
    this->refresh_elapsed = refresh_delay;

    this->item_colour=0;
    this->showcount=false;


    font_gap    = font.getHeight() + 4;
    max_strings = (int) ((display.height-top_gap-bottom_gap)/font_gap);
    incrementf   =0;
    root = SummNode();

    mouseover=false;

    right = (pos_x > (display.width/2)) ? true : false;

    if(this->title.size()) {
        this->top_gap+= font_gap;
    }
}

void Summarizer::mouseOut() {
    mouseover=false;
}

bool Summarizer::mouseOver(TextArea& textarea, vec2f mouse) {
    mouseover=false;

    if(right && mouse.x < pos_x) return false;
    if(mouse.y < top_gap || mouse.y > (display.height-bottom_gap)) return false;
    if(items.size()<1) return false;

    float y = mouse.y;

    std::list<SummItem>::iterator it;
    for(it=items.begin();it!=items.end();it++) {
        SummItem* si = &(*it);
        if(si->departing) continue;

        if(si->pos.y<=y && (si->pos.y+font.getHeight()+4) > y) {
            if(mouse.x< si->pos.x || mouse.x > si->pos.x + si->width) continue;

            std::vector<std::string> content;

            textarea.setText(si->unit.expanded);
            textarea.setColour(si->colour.truncate());
            textarea.setPos(mouse);
            mouseover=true;
            return true;
        }
    }

    return false;
}

void Summarizer::setColour(vec3f col) {
    this->item_colour = new vec3f(col);
}

bool Summarizer::isColoured() {
    return (item_colour!=0);
}

vec3f Summarizer::getColour() {
    return *item_colour;
}

Summarizer::~Summarizer() {
    if(item_colour!=0) delete item_colour;
}

bool Summarizer::supportedString(std::string& str) {
    return matchre.match(str);
}

void Summarizer::summarize() {
    strings.clear();
    root.summarize(strings,max_strings);

    size_t nostrs = strings.size();

    for(size_t i=0;i<nostrs;i++) {
        strings[i].buildSummary();
    }

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

    std::list<SummItem>::iterator it;

    //update summItems
    for(it=items.begin();it!=items.end();it++) {
        SummItem* item = &(*it);

        int match = -1;

        for(size_t j=0;j<nostrs;j++) {
            SummUnit summstr = strings[j];

            if(summstr.str.compare(item->unit.str) == 0) {
                item->updateUnit(summstr);

                match = (int)j;

                strfound[j]= true;

                break;
            }
        }

        if(match!= -1) {
            float destY = calcPosY(match);
            item->setDest(vec2f(pos_x, destY));
        } else {
            float destX = right ? (display.width + 100) : -100;
            item->setDest(vec2f(destX, item->pos.y), true);
        }
    }

    //add items for strings not found
    for(size_t i=0;i<nostrs;i++) {
        if(strfound[i]) continue;

        float startX = right ? display.width + 100 : -100;
        float destY  = getPosY(strings[i].str);

        items.push_back(SummItem(strings[i], vec2f(startX, destY), vec2f(pos_x, destY),pos_x, item_colour, font, showcount));
    }
}

void Summarizer::removeString(std::string& str) {
    root.removeWord(str);
    summarize();
}

float Summarizer::calcPosY(int i) {
    return top_gap + (incrementf * i) ;
}

int Summarizer::getBestMatch(std::string& str) {

    int bestdiff = -1;
    int best = -1;
    int bestsize = -1;

    size_t nostrs = strings.size();

    for(size_t i=0;i<nostrs;i++) {
        std::string strn = strings[i].str;
        std::string tmpstr = str;

        // if its not the same size, count against it
        int size_diff = strn.size() - tmpstr.size();

        if(size_diff>0) {
            strn = std::string(strn,0,tmpstr.size());
        }

        if(size_diff<0) {
            tmpstr = std::string(tmpstr,0,strn.size());
        }

        int truncated_diff = abs(strn.compare(tmpstr));

        //found it
        if(truncated_diff==0 && size_diff ==0) {
            best = i;
            break;
        }

        if(bestdiff==-1
            || truncated_diff<bestdiff
            || truncated_diff==bestdiff
               && tmpstr.size()>bestsize
               && strings[i].str.size()<str.size()) {
            best = i;
            bestdiff = truncated_diff;
            bestsize = tmpstr.size();
        }
    }

    return best;
}

std::string Summarizer::getBestMatchStr(std::string& str) {
    int pos = getBestMatch(str);
    return strings[pos].str;
}


float Summarizer::getMiddlePosY(std::string& str) {
    return getPosY(str) + (font.getHeight()) / 2;
}

float Summarizer::getPosY(std::string& str) {

    int best= getBestMatch(str);

    if(best!=-1) {
        return calcPosY(best);
    }

    return calcPosY(0);
}

void Summarizer::showCount(bool showcount) {
    this->showcount = showcount;
}

float Summarizer::addString(std::string& str) {
    root.addWord(str);
    summarize();

    return getMiddlePosY(str);
}

void Summarizer::logic(float dt) {

    refresh_elapsed+=dt;
    if(refresh_elapsed>=refresh_delay) {
        recalc_display();
        refresh_elapsed=0;
    }

    //move items
    std::list<SummItem>::iterator it;
    for(it = items.begin();it != items.end(); it++) {
        (*it).logic(dt);

        if((*it).destroy) {
            it = items.erase(it);
            if(it==items.end()) {
                break;
            }
        }
    }
}

void Summarizer::draw(float dt) {
   	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

    if(title.size()) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        font.draw((int)pos_x, (int)(top_gap - font_gap), title.c_str());
    }

    std::list<SummItem>::iterator it;
    for(it=items.begin();it!=items.end();it++) {
        (*it).draw();
    }

}
