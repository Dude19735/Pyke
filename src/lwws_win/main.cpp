#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <iostream>
#include <thread>
#include <string>
#include <typeinfo>
#include <typeindex>

#include "include/lwws_window_x11.hpp"
LWWS::LWWS_Window_X11* window = nullptr;

void run(){
    auto lwws_window = LWWS::LWWS_Window_X11("hello", 640, 480, true, false, 0, true);
    window = &lwws_window;

    window->windowEvents_Init();
    window->emit_windowEvent_Paint();

    while(window->windowEvents_Exist()){
        window->windowEvents_Pump();

        if(window->windowShouldClose()){
            break;
        }
    }
}

int main(int argc, char** argv) {
    // auto lwws_window = LWWS::LWWS_Window_X11("hello", 640, 480, true, false, 0, true);
    // auto window = &lwws_window;

    // window->windowEvents_Init();
    // window->emit_windowEvent_Paint();

    // while(window->windowEvents_Exist()){
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     window->emit_windowEvent_Paint();
    //     window->windowEvents_Pump();

    //     if(window->windowShouldClose()){
    //         break;
    //     }
    // }

    // std::cout << typeid(5).hash_code() << std::endl;
    // std::cout << typeid(5.0).hash_code() << std::endl;
    // std::cout << typeid(5.0f).hash_code() << std::endl;
    // std::cout << typeid('5').hash_code() << std::endl;
    // std::cout << typeid("5").hash_code() << std::endl;
    // std::cout << typeid(std::to_string(5)).hash_code() << std::endl;
    // std::cout << typeid(LWWS::LWWS_Key::Special::RandomKey).hash_code() << std::endl;

    // std::thread t = std::thread(run);
    // while(window == nullptr);
    // while(true){
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     // window->emit_windowEvent_Paint();
    //     // std::cout << "heartbeat" << std::endl;
    // }
    // t.join();
    // std::cout << "joined" << std::endl;
}