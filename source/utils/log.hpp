#ifndef GAMEZERO_UTILS_LOG_HPP
#define GAMEZERO_UTILS_LOG_HPP

#include <cstdio>

namespace GameZero{

#ifdef GAMEZERO_SETTING_GENERATE_LOG
#define LOG(severity, ...) { \
    printf("\n[ %s ] : GENERATED FROM FUNCTION[ %s ] LINE[ %i ] FILE[ %s ]", #severity, __FUNCTION__, __LINE__, __FILE__); \
    printf("\n\t"); \
    printf(__VA_ARGS__); \
    printf("\n\n"); \
}
#else
    #define LOG(...)
#endif//GAMEZERO_SETTING_GENERATE_LOG

}

#endif//GAMEZERO_UTILS_LOG_HPP