#ifndef GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP
#define GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP

#include "singleton.hpp"
#include <cstdlib>
#include <deque>
#include <functional>

namespace GameZero{

    /**
     * @brief Executes the registered functions at the end of program.
     *        The functions are called in reverse order in which they were
     *        registered.
     *        You can register destructors here.
     *        First registered function will be called at last.
     *        Registered functions must return void and take no argument
     */
    struct DestructionQueue : Singleton<DestructionQueue>{
        static inline std::deque<std::function<void()>> queue;
        
        /// Construct DestructionQueue.
        /// Automatically done when you do
        /// DestructionQueue::Get() or GetDestructionQueue().
        DestructionQueue(){
            std::atexit(Flush);
        }

        /// Register a function to destruction queue
        static inline void PushFunction(std::function<void()>&& func) noexcept{
            queue.push_back(func);
        }

        /// Call all registered functions.
        /// This is automatically done at main return or a normal exit.
        /// DANGER : Call this function manually at your own risk!!
        static inline void Flush() noexcept{
            // call the functions
            for(auto it = queue.rbegin(); it != queue.rend(); it++) (*it)();
            // clear the queue
            queue.clear();
        }
    };

    /// Get destruction queue pointer.
    /// Shorter than cleaner than doing : DestructionQueue::Get()->someCall().
    /// You can just do GetDestructionQueue()->someCall().
    /// Or you can store the destruction queue pointer locally
    /// if you have to use it multiple times
    inline DestructionQueue* GetDestructionQueue(){
        return DestructionQueue::Get();
    };

}

#endif//GAMEZERO_UTILS_DESTRUCTION_QUEUE_HPP