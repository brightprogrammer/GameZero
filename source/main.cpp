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

    while(window.isOpen){
        window.HandleEvents();
            
        renderer.cameraData.view = glm::lookAt(camera.position, (camera.position + camera.front), camera.up);
        renderer.cameraData.proj = glm::perspective(glm::radians(camera.fieldOfView), float(window.size.x) / float(window.size.y), 0.1f, 200.f);
        renderer.cameraData.proj[1][1] *= -1; // invert y

        renderer.Draw();
    }

    return 0;
}
