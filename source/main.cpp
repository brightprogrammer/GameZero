#include "gamezero.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include "utils/assert.hpp"
#include "vulkan/instance.hpp"
#include "vulkan/vulkan_core.h"
#include "window.hpp"
#include "app_state.hpp"
#include "camera.hpp"

#include <memory>
#include <stdexcept>
#include <chrono>
#include <unordered_map>

using namespace GameZero;

int main(){
    // create window
    Window window("GameZero - Editor", Vector2u(800, 600));

    // register window event callback
    window.RegisterWindowEventCallback([](WindowEventInfo& info){
        if(info.event == WindowEvent::Close) return false;
        return true;
    });


    ApplicationState *app = ApplicationState::Get();
    app->name = "GameZero";
    app->version = time(nullptr);
    app->enableValidation = true;
    app->mainWindow = &window;

    // create renderer for winodw
    Renderer renderer(window);

    // create camera
    Camera camera("main camera", window);

    // render loop time
    float deltaTime = 0.f;
    // number of frames to averge frame time over
    uint32_t frameSampleCount = 100;
    // keep track of frame number
    uint32_t frameNumber = 0;

    while(window.isOpen){
        auto loop_start_time = std::chrono::high_resolution_clock::now();       

        window.HandleEvents();
            
        renderer.cameraData.view = glm::lookAt(camera.position, (camera.position + camera.front), camera.up);
        renderer.cameraData.proj = glm::perspective(glm::radians(camera.fieldOfView), float(window.size.x) / float(window.size.y), 0.1f, 200.f);
        renderer.cameraData.proj[1][1] *= -1; // invert y

        renderer.Draw();

        auto loop_stop_time = std::chrono::high_resolution_clock::now();
        deltaTime += static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(loop_stop_time - loop_start_time).count());
    
        // show average frame time
        if((frameNumber % frameSampleCount) == 0){
            deltaTime /= frameSampleCount;
            printf("frame time : %fms\n", deltaTime);
            deltaTime = 0; // reset delta time
            frameNumber = 0; // reset frame number
        }
    }

    return 0;
}
