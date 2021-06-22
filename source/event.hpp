#ifndef GAMEZERO_EVENT_HPP
#define GAMEZERO_EVENT_HPP

#include "math/vector.hpp"
#include "utils/sdl_helper.hpp"

namespace GameZero{

    struct EventInfoBase{
        /// window that generated this event
        struct Window *window = nullptr;

        /// some user data
        void* userData = nullptr;
    };

    /// information about the window event and the window that triggered an event
    struct WindowEventInfo : public EventInfoBase{
        WindowEvent event;
    };

    /// information about the keyboard event and the window that triggered an event
    struct KeyboardEventInfo : public EventInfoBase{
        Keyboard key;
        KeyState state;
    };

    /// information about the mouse motion event and the window that triggered an event
    struct MouseMotionEventInfo : public EventInfoBase{
        Vector2f position;
        Vector2f relativePosition;  
    };

}

#endif//GAMEZERO_EVENT_HPP