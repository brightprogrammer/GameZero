file(GLOB_RECURSE GAMEZERO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(game_zero ${GAMEZERO_SOURCES})
target_link_directories(game_zero PUBLIC ${GAMEZERO_LOCAL_LIB_DIR})
target_link_libraries(game_zero vulkan SDL2 dl)
target_include_directories(game_zero PUBLIC ${GAMEZERO_LOCAL_INCLUDE_DIR})