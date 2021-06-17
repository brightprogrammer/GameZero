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

        /// should I enable vulkan validation layers?
        bool enableValidation = false;

        /// main window that this application will have
        struct Window* mainWindow {nullptr};
    };

    inline ApplicationState* GetApplicationState(){
        return ApplicationState::Get();
    }

    /// get the application's main window pointer.
    /// this is the window that you set in application state.
    inline struct Window* GetMainWindow(){
        return GetApplicationState()->mainWindow;
    }

    /// get the name of this application
    inline const char* GetApplicationName(){
        return GetApplicationState()->name;
    }

    inline const uint32_t GetApplicationVersion(){
        return GetApplicationState()->version;
    }
}

#endif//GAMEZERO_APP_STATE_HPP