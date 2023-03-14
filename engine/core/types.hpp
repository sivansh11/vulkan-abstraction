#ifndef CORE_TYPES_HPP
#define CORE_TYPES_HPP

#include <iostream>

namespace core {

struct Dimensions {
    union  {
        struct {
            uint32_t x, y, z;
        };
        struct {
            uint32_t width, height, depth;
        };
        struct {
            uint32_t data[3];
        };
    };
    
};

} // namespace core

#endif