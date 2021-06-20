#ifndef GAMEZERO_APP_STATE_HPP
#define GAMEZERO_APP_STATE_HPP

#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.hpp>

#include "utils.hpp"

namespace GameZero{

    /// game global information
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
