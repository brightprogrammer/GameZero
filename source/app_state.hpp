#ifndef GAMEZERO_APP_STATE_HPP
#define GAMEZERO_APP_STATE_HPP

#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.hpp>

#include "utils.hpp"

namespace GameZero{

    /**
     * @brief Application state contains some basic information
     *        about your application. Before making any call to
     *        GetVulkanState you MUST first fill in data to
     *        application state. Renderer calls GetVulkanInstance()
     *        so if you are creating a renderer then you must fill in
     *        valid data before that!
     *        
     *        
     * @note Every field has a default value except mainWindow.
     *       You must set window before calling
     */
    struct ApplicationState : public Singleton<ApplicationState>{
        /// application name
        const char* name = "app";

        /// application version
        uint32_t version = 0;

        /// main window
        struct Window* mainWindow {nullptr};

        /// should I enable vulkan validation layers?
        bool enableValidation = false;
    };

    inline ApplicationState* GetApplicationState(){
        return ApplicationState::Get();
    }

    /// get the name of this application
    inline const char* GetApplicationName(){
        return GetApplicationState()->name;
    }

    /// get application version from app state
    inline const uint32_t GetApplicationVersion(){
        return GetApplicationState()->version;
    }

    /// get main sdl window
    inline struct Window* GetMainWindow(){
        return GetApplicationState()->mainWindow;
    }

}

#endif//GAMEZERO_APP_STATE_HPP
