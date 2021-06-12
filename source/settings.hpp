#ifndef GAMEZERO_SETTINGS_HPP
#define GAMEZERO_SETTINGS_HPP

#include <cstdint>

#include "common.hpp"

namespace GameZero{
    
    constexpr static const char* GameZeroApplicationName = "GameZero";
    constexpr static uint32_t GameZeroVersionNumber = VK_MAKE_VERSION(0, 0, 0);

    #define GAMEZERO_SETTING_GENERATE_LOG 1
}

#endif//GAMEZERO_SETTINGS_HPP