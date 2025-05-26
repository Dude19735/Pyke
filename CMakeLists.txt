cmake_minimum_required (VERSION 3.22)

################################################################################################
# On Windows the compilation relies on environment variables. Make sure to check the 
# readme.md on this issue.
################################################################################################

set(MODULE_NAME_1 "_pyke")
set(PROJECT_NAME_1 "pyke")
project (vkviewer LANGUAGES CXX)

set(PYTHON_BUILD ON)
set(NB_TESTBUILD OFF)
if (PYTHON_BUILD)
    message("Set PYVK (${CMAKE_CURRENT_SOURCE_DIR})==============================")
    add_compile_definitions(PYVK)
endif()

# Properties
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_CXX_STANDARD 23)
    
if(WIN32)
    message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
    add_compile_options(/bigobj /std:c++latest) # /showIncludes)
elseif(UNIX)
    message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
    add_compile_options(-std=c++23 -lX11 -L/usr/X11R6/lib -I/usr/X)

    # no wayland yet because it apparently sucks on Nvidia
    # source: https://github.com/mykhani/hello-wayland
    # message("Wayland ============================================")
    # LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/lwws_win/cmake")
    # find_package(Wayland REQUIRED)
    # string(COMPARE EQUAL "${WAYLAND_LIBRARIES}" "" result)
    # if(result)
    #     message("Wayland not found! Make sure LIST(APPEND CMAKE_MODULE_PATH... points to the folder that contains FindWayland.cmake!")
    # else()
    #     message("Wayland libraries: ${WAYLAND_LIBRARIES}")
    #     message("Wayland includes:  ${WAYLAND_INCLUDE_DIRS}")
    # endif()

    message("X11 ==================================================")
    find_package(X11 REQUIRED)
    string(COMPARE EQUAL "${X11_INCLUDE_DIR}" "" result)
    if(result)
        message("Wayland not found! Make sure LIST(APPEND CMAKE_MODULE_PATH... points to the folder that contains FindWayland.cmake!")
    else()
        message("X11 libraries: ${X11_LIBRARIES}")
        message("X11 xcb libraries: ${X11_xcb_LIB}")
        message("X11 includes:  ${X11_INCLUDE_DIR}")
        message("X11 xcb includes: ${X11_xcb_INCLUDE_PATH}")
    endif()
endif()

message("Vulkan =============================================")
if(UNIX)
    message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
    find_package(Vulkan REQUIRED)
    message("Include dir:\t${Vulkan_INCLUDE_DIRS}")
    message("Link dir:\t\t${Vulkan_LIBRARIES}")
elseif(WIN32)
    message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
    # Retrieve the Vulkan SDK path from an environment variable
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    message("Vulkan SDK path: ${VULKAN_SDK}")
    set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include")
    set(Vulkan_LIBRARY "${VULKAN_SDK}/Lib")
    find_package(Vulkan REQUIRED)

    message("Include dir:\t${Vulkan_INCLUDE_DIRS}")
    message("Libraries dir:\t${Vulkan_LIBRARY}")
endif()

message("GLM =============================================")
if(WIN32)
    message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
    string(COMPARE EQUAL "$ENV{glm_INCLUDE_DIRS}" "" result)
    if(result)
        message("GLM environment variable not found!")
        message("Set 'glm_INCLUDE_DIRS' as environment variable in 'System Properties' under 'System varaibles' to the root path of the extracted zip folder")
        message("For example: glm_INCLUDE_DIR=C:\\Libraries\\glm")
    else()
        message("-- Found GLM at $ENV{glm_INCLUDE_DIRS}")
        SET(glm_INCLUDE_DIRS "$ENV{glm_INCLUDE_DIRS}")
    endif()
endif()

add_subdirectory(src)
add_subdirectory(tests)

if (PYTHON_BUILD)
    message("PYTHON_BUILD =============================================")
    if (SKBUILD)
        message("Module Python Build =============================================")
        # https://nanobind.readthedocs.io/en/latest/packaging.html
        # Try to import all Python components potentially needed by nanobind
        find_package(Python 3.12 
            REQUIRED COMPONENTS Interpreter Development.Module
            OPTIONAL_COMPONENTS Development.SABIModule)

        # Import nanobind through CMake's find_package mechanism
        find_package(nanobind CONFIG REQUIRED)

        if(NB_TESTBUILD)
            message("NB TEST_BUILD =============================================")
            nanobind_add_module("${MODULE_NAME_1}" NOMINSIZE LTO src/pyexp_nb_testmodule.cpp)
            target_include_directories("${MODULE_NAME_1}" PRIVATE Python)
            target_include_directories("${MODULE_NAME_1}" PRIVATE src/external/nanobind/include)
        else()
            message("Build Pyke =============================================")
            # We are now ready to compile the actual extension module
            nanobind_add_module("${MODULE_NAME_1}" STABLE_ABI NOMINSIZE LTO src/pyexp_vkviewer.cpp)
            target_include_directories("${MODULE_NAME_1}" PRIVATE Python)
            target_include_directories("${MODULE_NAME_1}" PRIVATE src/external/nanobind/include)
            if(UNIX)
                message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
                target_include_directories("${MODULE_NAME_1}" PRIVATE ${X11_INCLUDE_DIR})
                target_link_libraries("${MODULE_NAME_1}" PRIVATE ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
            elseif(WIN32)
                message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
                target_include_directories("${MODULE_NAME_1}" PRIVATE ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
                target_link_directories("${MODULE_NAME_1}" PRIVATE ${Vulkan_LIBRARY})
                target_link_libraries("${MODULE_NAME_1}" PRIVATE vulkan-1)
            endif()
        endif()

        nanobind_add_stub(
            "${MODULE_NAME_1}_stub"
            MODULE "${MODULE_NAME_1}"
            OUTPUT "${MODULE_NAME_1}.pyi"
            PYTHON_PATH $<TARGET_FILE_DIR:${MODULE_NAME_1}>
            DEPENDS "${MODULE_NAME_1}"
            MARKER_FILE py.typed
        )

        # Install directive for scikit-build-core
        # =======================================
        install(TARGETS "${MODULE_NAME_1}" LIBRARY DESTINATION ${PROJECT_NAME_1})
        install(FILES "${CMAKE_BINARY_DIR}/${MODULE_NAME_1}.pyi" "${CMAKE_BINARY_DIR}/py.typed" DESTINATION ${PROJECT_NAME_1})
    else()
        message("Regular Python Build =============================================")
        # src: https://nanobind.readthedocs.io/en/latest/building.html
        set(DEV_MODULE Development.Module)
        set(PYTHON_VERSION 3.12)
        find_package(Python ${PYTHON_VERSION} COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

        if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
            set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
            set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
        endif()

        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/external/nanobind)

        if(NB_TESTBUILD)
            nanobind_add_module("${MODULE_NAME_1}" NOMINSIZE LTO src/pyexp_nb_testmodule.cpp)
            target_include_directories("${MODULE_NAME_1}" PRIVATE Python)
            target_include_directories("${MODULE_NAME_1}" PRIVATE src/external/nanobind/include)
        else()
            nanobind_add_module("${MODULE_NAME_1}" NOMINSIZE LTO src/pyexp_vkviewer.cpp)
            target_include_directories("${MODULE_NAME_1}" PRIVATE Python)
            target_include_directories("${MODULE_NAME_1}" PRIVATE src/external/nanobind/include)
            if(UNIX)
                message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
                target_include_directories("${MODULE_NAME_1}" PRIVATE ${X11_INCLUDE_DIR})
                target_link_libraries("${MODULE_NAME_1}" PRIVATE ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
            elseif(WIN32)
                message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
                target_include_directories("${MODULE_NAME_1}" PRIVATE ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
                target_link_directories("${MODULE_NAME_1}" PRIVATE ${Vulkan_LIBRARY})
                target_link_libraries("${MODULE_NAME_1}" PRIVATE vulkan-1)
            endif()
        endif()
    endif()
else()
    message("CPP build =============================================")
    find_package(Python 3.12 REQUIRED COMPONENTS Interpreter)

    message("Boost ==============================================")
    find_package(Boost)
    message("${Boost_INCLUDE_DIRS}")

    if (PYTHON_BUILD)
        message("Set PYVK (${CMAKE_CURRENT_SOURC$E_DIR})==============================")
        add_compile_definitions(PYVK)
    endif()

    ##########################################################################################################
    # some primitive test for colors
    add_executable(test_terminal_colors "tests/test_terminal_colors.cpp")
    target_compile_definitions(test_terminal_colors PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
    target_include_directories(test_terminal_colors PRIVATE ${Boost_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

    ##########################################################################################################
    # extensive tests for the viewer
    add_executable(test_vk4_application "tests/test_vk4_application.cpp")
    target_compile_definitions(test_vk4_application PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
    target_include_directories(test_vk4_application PRIVATE ${Boost_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
    if(UNIX)
        message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
        target_include_directories(test_vk4_application PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(test_vk4_application ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
    elseif(WIN32)
        message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
        target_link_directories(test_vk4_application PRIVATE ${Vulkan_LIBRARY})
        target_link_libraries(test_vk4_application vulkan-1)
    endif()

    ##########################################################################################################
    # sample viewer
    add_executable(sample_viewer "sample_viewer.cpp")
    target_compile_definitions(sample_viewer PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
    target_include_directories(sample_viewer PRIVATE ${Boost_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
    if(UNIX)
        message("<<<<<<<<<<<<<<<<< Linux >>>>>>>>>>>>>>>>>")
        target_include_directories(sample_viewer PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(sample_viewer ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
    elseif(WIN32)
        message("<<<<<<<<<<<<<<<<< Windows >>>>>>>>>>>>>>>>>")
        target_link_directories(sample_viewer PRIVATE ${Vulkan_LIBRARY})
        target_link_libraries(sample_viewer PRIVATE vulkan-1)
    endif()

    # add_custom_command( # compile shaders and put them inside an h file
    #     OUTPUT "VK_ShaderBin"
    #     TARGET sample_viewer
    #     PRE_BUILD
    #     COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_renderer4/objects/compile_glsl2.py"
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # )

    ##########################################################################################################
    if(UNIX)
        # x11 test window
        add_executable(lwws "src/lwws_win/main.cpp")
        target_include_directories(
            lwws PRIVATE ${PROJECT_SOURCE_DIR} ${X11_INCLUDE_DIR} #${EGL_INCLUDE_DIRS}
        )
        target_link_libraries(
            lwws PRIVATE ${X11_LIBRARIES} #${EGL_LIBRARIES}
        )
    endif()
endif()