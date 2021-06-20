#ifndef GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP
#define GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP

#include <cstdlib>
#include <deque>
#include <functional>

#include "log.hpp"

namespace GameZero{

    /**
     * @brief Executes the registered functions at the end of program.
     *        The functions are called in reverse order in which they were
     *        registered.
     *        You can register destructors here.
     *        First registered function will be called at last.
     *        Registered functions must return void and take no argument
     */
    struct DestructionQueue{
        static inline std::deque<std::function<void()>> queue;
        
        /// Construct DestructionQueue.
        DestructionQueue() = default;

        /// Register a function to destruction queue
        static inline void PushFunction(std::function<void()>&& func) noexcept{
            queue.push_back(func);
        }

        /// Call all registered functions
        /// and clear the destruction queue
        static inline void Flush() noexcept{
            // call the functions
            for(auto it = queue.rbegin(); it != queue.rend(); it++) (*it)();
            // clear the queue
            queue.clear();
        }
    };

}

#endif//GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP
