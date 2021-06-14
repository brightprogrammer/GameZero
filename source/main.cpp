#include "SDL2/SDL_events.h"
#include "SDL2/SDL_keycode.h"
#include "gamezero.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "renderer.hpp"
#include "utils/assert.hpp"
#include "vulkan/vulkan_core.h"
#include "window.hpp"

#include <memory>
#include <unordered_map>

bool WindowEventCallback(const GameZero::WindowEvent& event, const GameZero::Window* window){
    if(event == GameZero::WindowEvent::Close) return false;
    return true;
}

int main(){
    GameZero::Window window(GameZero::GameZeroApplicationName, GameZero::Vector2u(800, 600));
    window.WindowEventCallback = WindowEventCallback;

    GameZero::Renderer renderer(window);

    // set window event callback
    // window.WindowEventCallback = WindowEventCallback;

    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 20.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat4 view;
    glm::vec3 direction;

    constexpr float playerSpeed = 0.2f;
    constexpr float sensitivity = 0.2f;
    float fov = 45.f;

    float yaw = -90.0f;
    float pitch = 89.f;

    while(window.isOpen){
        // window.Hand1leEvents();
        static SDL_Event event;
    
        while(SDL_PollEvent(&event)){
            if( (event.type == SDL_WINDOWEVENT) && (event.window.windowID == window.windowID) ){
            // check if window is closed
            if(event.window.event == SDL_WINDOWEVENT_CLOSE) window.isOpen = false;
        
        }
        // mouse motion event
        if(event.type == SDL_MOUSEMOTION){
            float xoffset = float(event.motion.xrel) * sensitivity;
            float yoffset = float(-event.motion.yrel) * sensitivity;
            
            yaw     += xoffset;
            pitch   += yoffset;

            if(pitch > 89.f) pitch = 89.f;
            if(pitch < -89.f) pitch = -89.f;

            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            camFront = glm::normalize(direction);
        }

        // key events
        if(event.type == SDL_KEYDOWN){
            if(event.key.keysym.sym == SDLK_UP){
                camPos += playerSpeed * camFront;
                // camPos.z += playerSpeed;
            }

            if(event.key.keysym.sym == SDLK_DOWN){
                camPos -= playerSpeed * camFront;
                // camPos.z -= playerSpeed;
            }

            if(event.key.keysym.sym == SDLK_RIGHT){
                camPos += (glm::normalize(glm::cross(camFront, camUp)) * playerSpeed);
                // camPos.x += playerSpeed;
            }

            if(event.key.keysym.sym == SDLK_LEFT){
                camPos -= glm::normalize(glm::cross(camFront, camUp)) * playerSpeed;
                // camPos.x -= playerSpeed;
            }

            if(event.key.keysym.sym == SDLK_w){
                camPos += playerSpeed * camUp;  
                // camPos.y += playerSpeed;
            }
                
            if(event.key.keysym.sym == SDLK_s){
                camPos -= playerSpeed * camUp;
                // camPos.y -= playerSpeed;
            }
        }

        if(event.type == SDL_MOUSEWHEEL){
            if(event.wheel.y > 0){
                fov += 0.1f;
            }else if(event.wheel.y < 0){
                fov -= 0.1f;
            }

            if(fov > 45.f) fov = 45.f;
            else if(fov < 1.f) fov = 1.f;
        }
        }
        
        
        renderer.cameraData.view = glm::lookAt(camPos, (camPos + camFront), camUp);
        renderer.cameraData.proj = glm::perspective(glm::radians(fov), float(window.size.x) / float(window.size.y), 0.1f, 100.f);
        renderer.cameraData.proj[1][1] *= -1;

        renderer.Draw();
    }
}