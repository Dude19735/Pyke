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
    std::vector<LWWS::LWWS_Viewport> viewports = {
        LWWS::LWWS_Viewport(20, 40, 300, 250)
    };

    auto lwws_window = LWWS::LWWS_Window_X11("hello", 640, 480, "#555555", viewports, true, false, 0, true);
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
    std::vector<LWWS::LWWS_Viewport> viewports = {
        LWWS::LWWS_Viewport(0, 0, 320, 100, 1, "#FF0000", "#FFFF00"),
        LWWS::LWWS_Viewport(180, 140, 150, 100, 1, "#00FF00", "#FF00FF"),
        LWWS::LWWS_Viewport(350, 260, 150, 100, 1, "#0000FF", "#00FFFF")
    };

    auto lwws_window = LWWS::LWWS_Window_X11("hello", 640, 480, "#555555", viewports, true, false, 0, false);
    auto window = &lwws_window;

    window->windowEvents_Init();
    window->emit_windowEvent_Paint();

    while(window->windowEvents_Exist()){
        // _device->vk_cleanSingleTimeCommands();
        window->windowEvents_Pump();

        if(window->windowShouldClose()){
            goto terminate;
        }
    }

    terminate:
    std::cout << "terminated" << std::endl;

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