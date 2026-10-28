#pragma once
#include <algorithm>

namespace inventory_next {
// Released Squared Away caps grid cells at 35 UI units. Fit the full projected
// width, including display-only gutters, without changing logical capacity.
inline int fitted_cell_body(int available_width,int columns,int spacing=2,int preferred=35) {
    if(available_width<1||columns<1||spacing<0)return 0;
    const int gaps=spacing*(columns-1);
    if(available_width<columns+gaps)return 0;
    return std::max(1,std::min({35,preferred,(available_width-gaps)/columns}));
}
}
