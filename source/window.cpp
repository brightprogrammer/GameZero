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

// initialize a sdl subsystem for first time only
inline void InitializeSDLSubSystem(uint32_t&& subSystemFlag){
    // initialize video subsystem if it isn;t initialized yet
    if(SDL_WasInit(subSystemFlag) != SDL_TRUE){ SDL_InitSubSystem(subSystemFlag); }
}

// default callback
inline bool DefaultWindowEventCallback(const GameZero::WindowEvent& winEvt, GameZero::Window* window){
    if(winEvt == GameZero::WindowEvent::Close) return false;
    return true;
}

// empty constructor
GameZero::Window::Window(){
    InitializeSDLSubSystem(SDL_INIT_VIDEO);
    // WindowEventCallback = DefaultWindowEventCallback;
}

// create window
GameZero::Window::Window(const char* windowTitle, const Vector2u& windowSize) :size(windowSize), title(windowTitle){
    InitializeSDLSubSystem(SDL_INIT_VIDEO);
    // WindowEventCallback = DefaultWindowEventCallback;
    SDL_WindowFlags windowFlags = SDL_WindowFlags(SDL_WINDOW_VULKAN);
    
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
    InitializeSDLSubSystem(SDL_INIT_VIDEO);
    size = windowSize;
    title = windowTitle;
    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;
    
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, windowFlags);
    ASSERT(window != nullptr, "FAILED TO CREATE WINDOW : [ Window Title %s ] ", SDL_GetError());
    
    // get window id
    windowID = SDL_GetWindowID(window);

    LOG(INFO, "Window created : %s", title)
}

// handle window events by using window event callback
void GameZero::Window::HandleEvents(){
    static SDL_Event event;
    
    // get all pending events
    // when there are no events, SDL_PollEvent returns 0
    // till that we have to get all events otherwise
    // we will recieve events per frame only!
    // which is a blundur
    while(SDL_PollEvent(&event)){
        // check window id for this window
        if(event.window.windowID == windowID){
            // handle window events
            if(event.type == SDL_WINDOWEVENT){
                // if window event callbacks is not empty then call all registered callbacks one by one
                if(!windowEventCallbacks.empty()){
                    for(const auto& [name, callback] : windowEventCallbacks){
                        isOpen = callback(WindowEvent(event.window.event), this);
                        if(!isOpen) return;
                    }
                }else{
                    // if no windowevent callback is registered then call default callback
                    DefaultWindowEventCallback(WindowEvent(event.window.event), this);
                }
            } // if(event.type == SDL_WINDOWEVENT)

            // handle keyboard events
            if(event.type == SDL_KEYDOWN){
                if(!keyboardEventCallbacks.empty()){
                    for(const auto& [name, callback] : keyboardEventCallbacks){
                        isOpen = callback(Keyboard(event.key.keysym.sym), true, this);
                        if(!isOpen) return;
                    }
                }
            } // if(event.type == SDL_KEYDOWN) 

            // handle keyboard events
            if(event.type == SDL_KEYUP){
                if(!keyboardEventCallbacks.empty()){
                    for(const auto& [name, callback] : keyboardEventCallbacks){
                        isOpen = callback(Keyboard(event.key.keysym.sym), false, this);
                        if(!isOpen) return;
                    }
                }
            } // if(event.type == SDL_KEYUP)

            // handle mouse motion events
            if(event.type == SDL_MOUSEMOTION){
                if(!mouseMotionEventCallbacks.empty()){
                    for(const auto& [name, callback] : mouseMotionEventCallbacks){
                        isOpen = callback(Vector2f(event.motion.x, event.motion.y), Vector2f(event.motion.xrel, event.motion.yrel), this);
                        if(!isOpen) return;
                    }
                }
            }

        } // if(event.window.windowID == windowID)
        else{
            // if this wasn't the window then resend the event
            SDL_PushEvent(&event);
        }
    }
}


// set window event callback
void GameZero::Window::RegisterWindowEventCallback(const char* callbackName, GameZero::Window::WindowEventCallbackPtr callback){
    windowEventCallbacks.insert(std::make_pair(callbackName, callback));
}

// set keyboard event callback
void GameZero::Window::RegisterKeyboardEventCallback(const char* callbackName, GameZero::Window::KeyboardEventCallbackPtr callback){
    keyboardEventCallbacks.insert(std::make_pair(callbackName, callback));
}

// set mouse motion event callback

void GameZero::Window::RegisterMouseMotionEventCallback(const char* callbackName, GameZero::Window::MouseMotionEventCallbackPtr callback){
    mouseMotionEventCallbacks.insert(std::make_pair(callbackName, callback));
}
