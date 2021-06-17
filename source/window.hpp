#ifndef GAMEZERO_WINDOW_HPP
#define GAMEZERO_WINDOW_HPP

#include "common.hpp"
#include "math.hpp"

namespace GameZero{
    
    /// window event
    enum WindowEvent : uint8_t{
        None        = SDL_WINDOWEVENT_NONE,
        Shown       = SDL_WINDOWEVENT_SHOWN,
        Hidden      = SDL_WINDOWEVENT_HIDDEN,
        Exposed     = SDL_WINDOWEVENT_EXPOSED,
        Moved       = SDL_WINDOWEVENT_MOVED,
        Resized     = SDL_WINDOWEVENT_RESIZED,
        SizeChanged = SDL_WINDOWEVENT_SIZE_CHANGED,
        Minimized   = SDL_WINDOWEVENT_MINIMIZED,
        Maximized   = SDL_WINDOWEVENT_MAXIMIZED,
        Restored    = SDL_WINDOWEVENT_RESTORED,
        Enter       = SDL_WINDOWEVENT_ENTER,
        Leave       = SDL_WINDOWEVENT_LEAVE,
        FocusGained = SDL_WINDOWEVENT_FOCUS_GAINED,
        FocusLost   = SDL_WINDOWEVENT_FOCUS_LOST,
        Close       = SDL_WINDOWEVENT_CLOSE,
        TakeFocus   = SDL_WINDOWEVENT_TAKE_FOCUS,
        HitTest     = SDL_WINDOWEVENT_HIT_TEST
    };

    /**
     * @note Window::surface is not created instantly.
     *
     *       Surface is created when vulkan instance is created
     *       i.e the first call to GetVulkanInstance()
     */

    /// Window
    struct Window{
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
        /// window event callback
        bool (*WindowEventCallback)(const WindowEvent& winEvent, const Window* window);

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