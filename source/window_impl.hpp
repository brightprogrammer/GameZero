#ifndef GAMEZERO_WINDOW_IMPL_HPP
#define GAMEZERO_WINDOW_IMPL_HPP

#include "window.hpp"

// set window event callback
template<>
void GameZero::Window::RegisterCallback<GameZero::Window::WindowEventCallbackPtr>(const char* callbackName, GameZero::Window::WindowEventCallbackPtr callback){
    windowEventCallbacks.insert(std::make_pair(callbackName, callback));
}

// set keyboard event callback
template<>
void GameZero::Window::RegisterCallback<GameZero::Window::KeyboardEventCallbackPtr>(const char* callbackName, GameZero::Window::KeyboardEventCallbackPtr callback){
    keyboardEventCallbacks.insert(std::make_pair(callbackName, callback));
}

// set mouse motion event callback
template<>
void GameZero::Window::RegisterCallback<GameZero::Window::MouseMotionEventCallbackPtr>(const char* callbackName, GameZero::Window::MouseMotionEventCallbackPtr callback){
    mouseMotionEventCallbacks.insert(std::make_pair(callbackName, callback));
}

#endif//GAMEZERO_WINDOW_IMPL_HPP
