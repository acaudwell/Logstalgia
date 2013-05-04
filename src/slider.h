#ifndef POS_SLIDER_H
#define POS_SLIDER_H

#include "core/logger.h"
#include "core/bounds.h"
#include "core/fxfont.h"

class PositionSlider {

    FXFont font;

    Bounds2D bounds;
    float percent;
    float mouseover;
    float mouseover_elapsed;
    float fade_time;
    float alpha;

    vec3 slidercol;

    float capwidth;
    std::string caption;
public:
    PositionSlider(float percent = 0.0f);

    void setColour(vec3 col);

    void setCaption(std::string cap);

    void setPercent(float percent);

    bool mouseOver(vec2 pos, float* percent_ptr);
    bool click(vec2 pos, float* percent_ptr);
    void logic(float dt);
    void draw(float dt);
};

#endif
