#include "gamezero.hpp"
#include "renderer.hpp"
#include "utils/assert.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <unordered_map>

// void WindowEventCallback(const GameZero::WindowEvent& event, const GameZero::Window* window){
// }

int main(){
    GameZero::Window window(GameZero::GameZeroApplicationName, GameZero::Vector2u(800, 600));
    
    GameZero::Renderer renderer(window);

    // set window event callback
    // window.WindowEventCallback = WindowEventCallback;

    while(window.isOpen){
        renderer.Draw();
        window.HandleEvents();
    }
}