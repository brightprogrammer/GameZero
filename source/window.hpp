#ifndef GAMEZERO_WINDOW_HPP
#define GAMEZERO_WINDOW_HPP

#include <SDL2/SDL.h>

#include <unordered_map>

#include "common.hpp"
#include "math.hpp"
#include "utils/sdl_helper.hpp"

namespace GameZero{
    
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

        /**
         * @brief Callback type to process keyboard events
         *        A keyboard event handler is already 
         * 
         * @param key : which keyboard key was pressed
         * @param keyDown : key down (true) or key up (false)
         * @param window : window that used this callback
         *
         * @return is window open ? 
         */
        typedef bool (*KeyboardEventCallbackPtr)(Keyboard key, bool keyDown, Window* window);

        /**
         * @brief Callback type to process window events
         * 
         * @param windowEvent : which window event happened
         * @param window : window that used this callback
         *
         * @return is window open ? 
         */
        typedef bool (*WindowEventCallbackPtr)(const WindowEvent& windowEvent, Window* window);

        /**
         * @brief Callback type to process mouse motion events.
         *        This is set by camera automatically if a camera
         *        uses this window.
         *        Setting this to some manual function other than camera
         *        can help in creating custom camera.
         *
         * @param position : current position of mouse.
         * @param relativePosition : relative position of mouse with respect
         *        to last mouse position.
         * @param window : the window that used this callback.
         * 
         * @return is window open ? 
         */
        typedef bool (*MouseMotionEventCallbackPtr)(const Vector2f& position, const Vector2f& relativePosition, Window* window);
        
        /// list of all registered window event callbacks
        std::unordered_map<const char*, WindowEventCallbackPtr> windowEventCallbacks;
        /// list of all registered keyboard event callbacks
        std::unordered_map<const char*, KeyboardEventCallbackPtr> keyboardEventCallbacks;
        /// list of all registered mouse motion event callbacks
        std::unordered_map<const char*, MouseMotionEventCallbackPtr> mouseMotionEventCallbacks;

        /**
         * @brief Register a new callback to window
         * 
         * @tparam callbackType : type of callback (eg:WindowEventCallbackPtr, KeyboardEventCallbackPtr, etc...)
         * @param callbackName : name of callback for querying callbacks.
         *        You can set this to who registers the callback.
         *        Callbacks registered by a camera will be named same as camera name.
         * @param callback : callback function pointer with matching signature as that of callback type.
         */
        template<typename callbackType>
        void RegisterCallback(const char* callbackName, callbackType callback);

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

// include implementation
#include "window_impl.hpp"

#endif//GAMEZERO_WINDOW_HPP
