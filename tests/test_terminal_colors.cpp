#define BOOST_TEST_MODULE RunTestTerminalColor
#include "boost/test/included/unit_test.hpp"

#include <iostream>
#include <typeinfo>
#include <string>
#include <tuple>
#include <format>

#include "../src/Vk_Coloring.hpp"
#include "../src/Vk_Logger.hpp"

BOOST_AUTO_TEST_SUITE(RunTestTerminalColor)

BOOST_AUTO_TEST_CASE(VulkanRenderer_TestTerminalColor, *boost::unit_test::disabled())
{
    printf("\n");
    printf("\x1B[31mTexting\033[0m\t\t");
    printf("\x1B[32mTexting\033[0m\t\t");
    printf("\x1B[33mTexting\033[0m\t\t");
    printf("\x1B[34mTexting\033[0m\t\t");
    printf("\x1B[35mTexting\033[0m\n");

    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[36mTexting\033[0m\t\t");
    printf("\x1B[37mTexting\033[0m\t\t");
    printf("\x1B[93mTexting\033[0m\n");

    printf("\033[3;42;30mTexting\033[0m\t\t");
    printf("\033[3;43;30mTexting\033[0m\t\t");
    printf("\033[3;44;30mTexting\033[0m\t\t");
    printf("\033[3;104;30mTexting\033[0m\t\t");
    printf("\033[3;100;30mTexting\033[0m\n");

    printf("\033[3;41;97mTexting\033[0m\t\t");
    printf("\033[3;47;35mTexting\033[0m\t\t");
    printf("\033[3;47;35mTexting\033[0m\t\t");
    printf("\t\t");
    printf("\n");

    std::cout << "=================================" << std::endl;
    std::cout << VK4::GlobalCasters::castTraceTitle("Trace") << std::endl;
    std::cout << VK4::GlobalCasters::castLogTitle("Log") << std::endl;
    std::cout << VK4::GlobalCasters::castMessageTitle("Message") << std::endl;
    std::cout << VK4::GlobalCasters::castWarnTitle("Warn") << std::endl;
    std::cout << VK4::GlobalCasters::castErrorTitle("Error") << std::endl;
    std::cout << VK4::GlobalCasters::castFatalTitle("Fatal") << std::endl;

    std::cout << VK4::GlobalCasters::castConstructorTitle("Constructor Title") << std::endl;
    std::cout << VK4::GlobalCasters::castDestructorTitle("Destructor Title") << std::endl;

    std::cout << VK4::GlobalCasters::castHighlightCyan("Highlight Cyan") << std::endl;
    std::cout << VK4::GlobalCasters::castHighlightYellow("Highlight Yellow") << std::endl;
    std::cout << VK4::GlobalCasters::castHighlightRed("Highlight Red") << std::endl;
    std::cout << VK4::GlobalCasters::castHighlightGreen("Highlight Green") << std::endl;
}

class TestClass {
    public:
    TestClass(){
        VK4::Vk_Logger::Trace(typeid(this), "Create Test object");
    }
};

BOOST_AUTO_TEST_CASE(VulkanRenderer_TestVkLogger /*, *boost::unit_test::disabled()*/)
{
    TestClass c;
    VK4::Vk_Logger::Trace(typeid(this), "Trace, no args");
    VK4::Vk_Logger::Trace(typeid(this), "Trace, int arg {}", 1);
    VK4::Vk_Logger::Trace(typeid(this), "Trace, str arg {}", "str");
    VK4::Vk_Logger::Trace(typeid(this), "Trace, str {} arg {}", "str", 1);
    VK4::Vk_Logger::Trace(typeid(this), "Trace, {} str {} arg {}", "str", 1, 2.0);

    VK4::Vk_Logger::Log(typeid(this), "Log, no args");
    VK4::Vk_Logger::Log(typeid(this), "Log, int arg {}", 1);
    VK4::Vk_Logger::Log(typeid(this), "Log, str arg {}", "str");
    VK4::Vk_Logger::Log(typeid(this), "Log, str {} arg {}", "str", 1);
    VK4::Vk_Logger::Log(typeid(this), "Log, {} str {} arg {}", "str", 1, 2.0);

    VK4::Vk_Logger::Message(typeid(this), "Message, no args");
    VK4::Vk_Logger::Message(typeid(this), "Message, int arg {}", 1);
    VK4::Vk_Logger::Message(typeid(this), "Message, str arg {}", "str");
    VK4::Vk_Logger::Message(typeid(this), "Message, str {} arg {}", "str", 1);
    VK4::Vk_Logger::Message(typeid(this), "Message, {} str {} arg {}", "str", 1, 2.0);

    VK4::Vk_Logger::Warn(typeid(this), "Warn, no args");
    VK4::Vk_Logger::Warn(typeid(this), "Warn, int arg {}", 1);
    VK4::Vk_Logger::Warn(typeid(this), "Warn, str arg {}", "str");
    VK4::Vk_Logger::Warn(typeid(this), "Warn, str {} arg {}", "str", 1);
    VK4::Vk_Logger::Warn(typeid(this), "Warn, {} str {} arg {}", "str", 1, 2.0);

    VK4::Vk_Logger::Error(typeid(this), "Error, no args");
    VK4::Vk_Logger::Error(typeid(this), "Error, int arg {}", 1);
    VK4::Vk_Logger::Error(typeid(this), "Error, str arg {}", "str");
    VK4::Vk_Logger::Error(typeid(this), "Error, str {} arg {}", "str", 1);
    VK4::Vk_Logger::Error(typeid(this), "Error, {} str {} arg {}", "str", 1, 2.0);

    //VK4::Vk_Logger::Fatal(typeid(this), "Fatal, no args");
}

BOOST_AUTO_TEST_SUITE_END()