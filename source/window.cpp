#include "window.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_error.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL_video.h"
#include "utils.hpp"
#include "math.hpp"
#include "vulkan/instance.hpp"
#include "vulkan/vulkan_core.h"
#include <cstdio>

inline void initSDLVideo(){
    // initialize video subsystem
    if(SDL_WasInit(SDL_INIT_VIDEO)){ SDL_InitSubSystem(SDL_INIT_VIDEO); }
}

// default callback
inline bool DefaultWindowEventCallback(const GameZero::WindowEvent& winEvt, const GameZero::Window* window){
    if(winEvt == GameZero::Close) return false;
    return true;
}

// empty constructor
GameZero::Window::Window(){
    initSDLVideo();
    WindowEventCallback = DefaultWindowEventCallback;
}

// create window
GameZero::Window::Window(const char* windowTitle, const Vector2u& windowSize) : title(windowTitle), size(windowSize){
    initSDLVideo();
    WindowEventCallback = DefaultWindowEventCallback;
    SDL_WindowFlags windowFlags = SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
    
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, windowFlags);
    ASSERT(window != nullptr, "FAILED TO CREATE WINDOW : [ Window Title %s ] ", SDL_GetError());

    // get window id
    windowID = SDL_GetWindowID(window);

    LOG(INFO, "Window created : %s", title);
}

// destroy window
GameZero::Window::~Window(){
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    LOG(INFO, "Window destroyed : %s", title);
}

// create window
void GameZero::Window::Create(const char *windowTitle, const Vector2u &windowSize){
    initSDLVideo();
    size = windowSize;
    title = windowTitle;
    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;
    
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, windowFlags);
    ASSERT(window != nullptr, "FAILED TO CREATE WINDOW : [ Window Title %s ] ", SDL_GetError());
    
    // get window id
    windowID = SDL_GetWindowID(window);

    LOG(INFO, "Window created : %s", title)
}

void GameZero::Window::HandleEvents(){
    static SDL_Event event;
    
    SDL_PollEvent(&event);
    if( (event.type == SDL_WINDOWEVENT) && (event.window.windowID == windowID) ){
        isOpen = WindowEventCallback(WindowEvent(event.window.event), this);
    }
}