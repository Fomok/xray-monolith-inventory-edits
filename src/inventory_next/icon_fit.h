#pragma once

#include <algorithm>

namespace inventory_next {
struct IconFit { float x=0,y=0,w=0,h=0; };

// Fit atlas pixels inside the occupied UI rectangle in the final screen metric.
// Rotation changes the displayed aspect, never the atlas crop or grid footprint.
inline IconFit fit_icon(float source_w,float source_h,float box_w,float box_h,
                        float screen_x,float screen_y,bool rotated) {
    if(source_w<=0||source_h<=0||box_w<=0||box_h<=0||screen_x<=0||screen_y<=0)return {};
    const float aspect=rotated?source_h/source_w:source_w/source_h;
    const float pixel_w=std::min(box_w*screen_x,box_h*screen_y*aspect);
    const float pixel_h=pixel_w/aspect;
    const float width=pixel_w/screen_x,height=pixel_h/screen_y;
    return {(box_w-width)*0.5f,(box_h-height)*0.5f,width,height};
}
}
