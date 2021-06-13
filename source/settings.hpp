#ifndef GAMEZERO_SETTINGS_HPP
#define GAMEZERO_SETTINGS_HPP

#include <cstdint>

#include "common.hpp"

namespace GameZero{
    
    constexpr static const char* GameZeroApplicationName = "GameZero";
    constexpr static uint32_t GameZeroVersionNumber = VK_MAKE_VERSION(0, 0, 0);

    // enable double buffering
    #define GAMEZERO_ENABLE_DOUBLE_BUFFERING

    /// dictates framebuffering level : double, triple etc
    constexpr static size_t FrameOverlapCount =
    #ifdef GAMEZERO_ENABLE_DOUBLE_BUFFERING
        2;
    #elif GAMEZERO_ENABLE_TRIPLE_BUFFERING
        3;
    #else
        1;
    #endif

    #define GAMEZERO_SETTING_GENERATE_LOG 1
}

#endif//GAMEZERO_SETTINGS_HPP