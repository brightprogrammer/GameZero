#ifndef GAMEZERO_SINGLETON_HPP
#define GAMEZERO_SINGLETON_HPP

namespace GameZero{

    template<class Derived>
    class Singleton{
    protected:
        Singleton() = default;
        ~Singleton() = default;

    private:
        /// pointer to derived    
        static inline Derived *derived = nullptr;
    
    public:
        // /// get singleton instance after constructing it
        // template<typename ...arg_types>
        // static inline Derived* Get(arg_types... args){
        //     if(!derived) derived = new Derived(args...);
        //     return derived;
        // }
        
        /// get singleton instance
        static inline Derived* Get(){
            if(!derived) derived = new Derived();
            return derived;
        }

        // delete copy constructors
        Singleton& operator = (const Singleton&) = delete;
        Singleton& operator = (Singleton&&) = delete;
    };

}

#endif//GAMEZERO_SINGLETON_HPP