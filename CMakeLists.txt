cmake_minimum_required (VERSION 3.22)

################################################################################################
# On Windows the compilation relies on environment variables. Make sure to check the 
# readme.md on this issue.
################################################################################################

project (vkviewer)

# add_compile_definitions(PYVK "1")

# Properties
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(WIN32)
    add_compile_options(/bigobj /std:c++latest) # /showIncludes)
elseif(UNIX)
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

message("Boost ==============================================")
find_package(Boost)
message("${Boost_INCLUDE_DIRS}")

message("Vulkan =============================================")
find_package(Vulkan)
if(UNIX)
    message("Include dir:\t${Vulkan_INCLUDE_DIRS}")
    message("Link dir:\t\t${Vulkan_LIBRARIES}")
elseif(WIN32)
    message("Include dir:\t${Vulkan_INCLUDE_DIRS}")
    string(FIND "${Vulkan_LIBRARIES}" "/vulkan-1.lib" vulkan_pos REVERSE)
    string(SUBSTRING "${Vulkan_LIBRARIES}" 0 ${vulkan_pos} Vulkan_LIBRARIES)
    message("Link dir:\t\t${Vulkan_LIBRARIES}")
endif()

message("GLM =============================================")
if(WIN32)
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

##########################################################################################################
# some primitive test for colors
add_executable(test_terminal_colors "tests/test_terminal_colors.cpp")
target_compile_definitions(test_terminal_colors PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
target_include_directories(test_terminal_colors PRIVATE ${Boost_INCLUDE_DIRS})

##########################################################################################################
# extensive tests for the viewer
add_executable(test_vk4_application "tests/test_vk4_application.cpp")
target_compile_definitions(test_vk4_application PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
target_include_directories(test_vk4_application PRIVATE ${Boost_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
# target_link_libraries(test_vk4_application ExternalLib)
if(UNIX)
    target_include_directories(test_vk4_application PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(test_vk4_application ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
elseif(WIN32)
    target_link_directories(test_vk4_application PRIVATE ${Vulkan_LIBRARIES})
    target_link_libraries(test_vk4_application vulkan-1)
    add_custom_command( # compile shaders and put them inside an h file
        TARGET test_vk4_application
        PRE_BUILD
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_renderer4/objects/compile_glsl2.py"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

##########################################################################################################
# sample viewer
add_executable(sample_viewer "sample_viewer.cpp")
target_compile_definitions(sample_viewer PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>) # this actually works (adds _DEBUG=1 if the setting is debug mode)
target_include_directories(sample_viewer PRIVATE ${glm_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})
# target_link_libraries(sample_viewer ExternalLib)
if(UNIX)
    target_include_directories(sample_viewer PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(sample_viewer ${Vulkan_LIBRARIES} ${X11_LIBRARIES} ${X11_xcb_LIB})
elseif(WIN32)
    target_link_directories(sample_viewer PRIVATE ${Vulkan_LIBRARIES})
    target_link_libraries(sample_viewer vulkan-1)
    add_custom_command( # compile shaders and put them inside an h file
        TARGET sample_viewer
        PRE_BUILD
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_renderer4/objects/compile_glsl2.py"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

##########################################################################################################
if(UNIX)
    # x11 test window
    add_executable(lwws "src/lwws_win/main.cpp")
    target_include_directories(
        lwws PRIVATE ${PROJECT_SOURCE_DIR} ${X11_INCLUDE_DIR} #${EGL_INCLUDE_DIRS}
    )
    target_link_libraries(
        lwws ${X11_LIBRARIES} #${EGL_LIBRARIES}
    )
endif()