#ifndef GAMEZERO_WINDOW_HPP
#define GAMEZERO_WINDOW_HPP

#include <SDL2/SDL.h>

#include <deque>
#include <functional>

#include "common.hpp"
#include "math.hpp"
#include "event.hpp"

namespace GameZero{
    
    using KeyboardEventCallback = std::function<bool(KeyboardEventInfo&)>;
    using WindowEventCallback = std::function<bool(WindowEventInfo&)>;
    using MouseMotionEventCallback = std::function<bool(MouseMotionEventInfo&)>;
        

    /**
     * @note Window::surface is not created instantly.
     *
     *       Surface is created when vulkan instance is created
     *       i.e the first call to GetVulkanInstance()
     */

    /// Window
    class Window{
    public:
        /// sdl window handle
        SDL_Window* window = nullptr;
        /// size of this window
        Vector2u size;
        /// this window title
        const char* title;
        /// check if window is open
        bool isOpen = true;
        /// sdl window id
        int windowID;

        /// empty constructor
        Window();

        /**
         * @brief Construct a new Window object
         * 
         * @param title 
         * @param size 
         */
        Window(const char* title, const Vector2u& size);
        
        /**
         * @brief Destroy the Window object
         * 
         */
        ~Window();

        /// list of all registered window event callbacks
        std::deque<WindowEventCallback> windowEventCallbacks;
        /// list of all registered keyboard event callbacks
        std::deque<KeyboardEventCallback> keyboardEventCallbacks;
        /// list of all registered mouse motion event callbacks
        std::deque<MouseMotionEventCallback> mouseMotionEventCallbacks;

        void RegisterWindowEventCallback(WindowEventCallback&& callback){
            windowEventCallbacks.push_back(callback);
        }

        void RegisterKeyboardEventCallback(KeyboardEventCallback&& callback){
            keyboardEventCallbacks.push_back(callback);
        }

        void RegisterMouseMotionEventCallback(MouseMotionEventCallback&& callback){
            mouseMotionEventCallbacks.push_back(callback);
        }

        /**
         * @brief Create Window
         * 
         * @param title 
         * @param size 
         */
        void Create(const char* title, const Vector2u& size);
        
        /**
         * @brief Get the Window Extent 
         * 
         * @return VkExtent2D 
         */
        inline VkExtent2D GetExtent() const{
            return VkExtent2D{size.x, size.y};
        }

        /// handle recieved events
        void HandleEvents();
    };
}

#endif//GAMEZERO_WINDOW_HPP
