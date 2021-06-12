#ifndef GAMEZERO_MATH_VECTOR_HPP
#define GAMEZERO_MATH_VECTOR_HPP

#include <cstdint>

namespace GameZero{
    
    template<typename T>
    struct Vector2{
        Vector2(const T& x = 0, const T& y = 0) : x(x), y(y){}
        T x;
        T y;
    };

    typedef Vector2<uint32_t> Vector2u;
    typedef Vector2<int32_t> Vector2i;
    typedef Vector2<float> Vector2f;

}

#endif//GAMEZERO_MATH_VECTOR_HPP