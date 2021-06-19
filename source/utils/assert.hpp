#ifndef GAMEZERO_UTILS_ASSERT_HPP
#define GAMEZERO_UTILS_ASSERT_HPP

#include <cstdio>
#include "vk_result_string.hpp"

namespace GameZero{

/// to assert with message
#define ASSERT(condition, ...) \
    if(!(condition)){ \
        printf("\nASSERT FAILURE [ %s ]\n", #condition); \
        printf("ERROR RAISED FROM FUNCTION[ %s ] LINE[ %i ] FILE[ %s ]", __FUNCTION__, __LINE__, __FILE__); \
        printf("\n\t"); \
        printf(__VA_ARGS__); \
        printf("\n\n"); \
        exit(-1); \
    }

/// to check vk result values
#define CHECK_VK_RESULT(statement, ...) {\
    vk::Result res = statement; \
    if(res != vk::Result::eSuccess){ \
        printf("\nASSERT FAILURE [ %s ]\n", #statement); \
        printf("ERROR RAISED FROM FUNCTION[ %s ] LINE[%i] FILE[%s]", __FUNCTION__, __LINE__, __FILE__); \
        printf("\n\t"); \
        printf(__VA_ARGS__); \
        printf("\n\t VULKAN RESULT MESSAGE : %s", VkResultString(res)); \
        printf("\n\n"); \
        exit(-1); \
    }}

}

#endif//GAMEZERO_UTILS_ASSERT_HPP