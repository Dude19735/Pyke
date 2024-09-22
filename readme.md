# Introduction
This is a 3D viewer based on the Vulkan API with Python bindings. **Note:** this is very much a work in progress, so it may be a bit unstable because asynchronous programs are hard ^^. There will also be substantial changes in the future, potentially also in the structure of the files and folders. But the viewer in it's current state generally works.

The viewer should work on Linux and Windows and has custom windowing systems based on the Windows API and X11. It has never been tested on a Mac and will probably not work there out-of-the-box.

<div style="text-align: center">
   <img src="./png/screenshot.png" style="width: 70%;">
   <figcaption></figcaption>
   <br>
</div>

# Intention
The goal is to create a fully asynchronous viewer where data can be transfered between CPU and GPU and debug points can be set inside the program where the viewer is used to visualize things without blocking the rendering process. At the end, the viewer should allow someone to use for example numpy arrays to deal with 3D objects in Python and directly render them into a nice, representable scene using a slim and uncomplicated API.

Currently points, lines and meshes are supported with corresponding update methods. There are not lights yet, but they will be added at some point. The format of the objects currently seperates vertices, colors and indices which is good to update only one of the three at any given time. Other combinations of data are technically already added but not supported yet.

There is no mechanism to click on objects and get the 3D points underneath and there is no support for text labels yet.

The viewer supports multiple viewports inside one single framebuffer with either object centric or viewing focused cameras. Objects can be attached and detached to individual viewports during runtime. Technically, everything relevant should be thread safe, but since it's a work in progress, you never know ^^.

The viewer supports both registering callback functions and start-stop style usage as depicted below
```Python
   # start viewer outside of main thread
   while(viewer.is_running()):
      # do things here that may need debugging
   # ...stop viewer
```

Python bindings can be created, but the script may have to be customized. There is no pip module yet. Check out setup_vkviewer.py. Instructions are included in the comments at the beginning of the script.

# Examples
Check out sample_viewer.cpp for an example on how the viewer works in C++ and test_py/test_viewer.py for a sample script on how the viewer works with the Python bindings.

# Installation
1. Follow the instructions in *Build requirements* for your platform (Windows, Linux)
2. Follow instructions on *How to build for C++* or *How to export for Python*

# Build requirements
VSCode is a good 'works everywhere' platform. Thus the build process is described with this platform.

---
## Linux

Install a C++ compiler first. Best pick the newest versions.
```bash
sudo apt-get install gcc-14
sudo apt-get install g++-14
```

#### X11/Wayland
Currently only X11 is supporte (subject to change).
```bash
sudo apt-get install libx11-dev libxpm-dev libxft-dev libxext-dev mesa-common-dev
```

#### Download Visual Studio Code
1. Goto https://visualstudio.microsoft.com/downloads/ (provided the link is still the same) and download and install the Linux version of VSCode.
2. Do the same as in point 2,3 of the Windows description below.

#### Download Vulkan SDK
* Follow the instructions on https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html

#### Download Boost
* sudo apt-get install libboost-all-dev

#### Download GLM
* sudo apt-get install libglm-dev

#### Download Pybind11
* **Note:** this is only necessary to create the Python bindings. Make sure to also install the pybind-stub generator as outlined in the setup script.
* create a virtual environment for Python. For example
```bash
$> sudo apt install python3-virtualenv
$> mkdir /home/lol/.venvs
$> virtualenv /home/lol/.venvs/standard
# Activate virtual environment:
$> source /home/lol/.venvs/standard/bin/activate
```
* run *pip install pybind11* from a terminal inside that virtual environment

**NOTE**: for Python 3.12

---

## Windows
#### Download Visual Studio and Visual Studio Code
1. Goto https://visualstudio.microsoft.com/downloads/ (provided the link is still the same, otherwise use a common internet search machine) and download the latest Visual Studio Community and Visual Studio Code versions. For Windows, installing Visual Studio is the least cumbersome method of installing a C++ compiler. Since you do the Windows build, you automatically have enough storage space on your disk. Also install VSCode because it's increasingly nicer to work with that one as opposed to Visual Studio unless you use C# or other exclusively Microsoft products.
   <div style="text-align: center">
      <img src="./png/vs-and-vscode-install-1.png" style="width: 50%;">
      <figcaption></figcaption>
      <br>
   </div>
2. Install CMake: goto cmake.org/download/ and get the latest Windows binaries
   <div style="text-align: center">
      <img src="./png/cmake-install-0.png" style="width: 30%;">
      <figcaption></figcaption>
      <br>
   </div>   
3. Add the CMake extensions for VSCode. 
   <div style="text-align: center">
      <img src="./png/cmake-install.png" style="width: 50%;">
      <figcaption></figcaption>
      <br>
   </div>

   Right after installing *CMake Tools*, VSCode will prompt some configuration for visibility. Follow the prompt and set **Visibility** to **visible**. This will allways show the build options at the bottom of VSCode.
   <div style="text-align: center">
      <img src="./png/cmake-install-2.png" style="width: 30%;">
      <img src="./png/cmake-install-3.png" style="width: 30%;">
      <img src="./png/cmake-install-4.png" style="width: 100%;">
      <figcaption></figcaption>
      <br>
   </div>

#### Download Vulkan SDK
1. Goto https://www.lunarg.com/vulkan-sdk/ and download the newest SDK
   <div style="text-align: center">
      <img src="./png/vulkan-install-1.png" style="width: 50%;">
      <figcaption></figcaption>
      <br>
   </div>
2. Follow the instructions on https://vulkan.lunarg.com/doc/sdk/1.3.283.0/windows/getting_started.html (especially the **Install the SDK** part)
3. Add environment variables for the **validation layers**. Make sure to adjust for the correct version!
   ```
   C:\> set VK_LAYER_PATH=C:\Libraries\VulkanSDK\1.3.211.0\Bin
   C:\> set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation
   ```
4. Run the following to see if the installation works and your system supports Vulkan
   ```
   C:\> vkcube
   ```
5. CMakeLists.txt configuration should now find the correct boost installation. Test by **deleting CMAKE cache and reconfigure**.

#### Download Boost
(Source: https://www.geeksforgeeks.org/how-to-install-c-boost-libraries-on-windows/)
1. Goto boost.org and download the newest Windows binaries
2. Create a folder **C:\Boost**
3. Extract the downloaded zip folder into **C:\Boost**. This will take a while. Go get a coffee (provided you drink such things. Otherwise go get a cup of tea)! Make sure, the folder looks like on the image below. **Not** C:\Boost\boost_1_85_0 or similar.
   <div style="text-align: center">
      <img src="./png/boost-install-1.png" style="width: 20%;">
      <figcaption></figcaption>
      <br>
   </div>
4. Create a new environment variable **Boost_INCLUDE_DIR=C:\Boost\include** (or to wherever else you put the boost files)
5. Maybe reboot the system or not (as usual in Windows). **Note:** more environment variables follow. Reboot the system (or not) at the end of the installation instructions.
6. CMakeLists.txt configuration should now find the correct boost installation. Test by **deleting CMAKE cache and reconfigure**. (**Note**: it is not necessary to add library link path here because the required parts of Boost are header-only)

<!-- #### Download GLFW3
1. Goto https://www.glfw.org/download and download the newest 64 bit Windows binaries
   <div style="text-align: center">
      <img src="./png/glfw-install-1.png" style="width: 20%;">
      <figcaption></figcaption>
      <br>
   </div>
2. Extract the zip folder to a random location
3. Copy the extracted folder to a random location (preferably somewhere on C:\Libraries)
4. Create a Windows environment variable **glfw3_DIR=[path from above, i.e. C:\Libraries...]
   <div style="text-align: center">
      <img src="./png/glfw-install-2.png" style="width: 20%;">
      <img src="./png/glfw-install-3.png" style="width: 20%;">
      <figcaption></figcaption>
      <br>
   </div>
5. Rename the folder in the GLFW root folder that corresponts to the installed Visual Studio version from **lib-vc20xx** to **lib**.
   <div style="text-align: center">
      <img src="./png/glfw-install-4.png" style="width: 45%;">
      <img src="./png/glfw-install-5.png" style="width: 45%;">
      <figcaption></figcaption>
      <br>
   </div>
6. Maybe reboot the system or not (as usual in Windows)
7. The CmakeLists.txt configuration should now set the correct paths for compilation. Test by **deleting CMAKE cache and reconfigure**. -->

#### Download GLM
1. Godo https://glm.g-truc.net. You will be redirected to github.
2. Select the latest, highlighted release and then download glm-[version]-light.zip
   <div style="text-align: center">
      <img src="./png/glm-install-0.png" style="width: 45%;">
      <img src="./png/glm-install-1.png" style="width: 45%;">
      <figcaption></figcaption>
      <br>
   </div>
3. Extract the contents into a random target location, for example C:\Libraries\glm\glm. **Note** that **glm** appears **twice**. This is intentional and will result in
   ```
   #include "glm/glm.hpp"
   #include "glm/gtc/matrix_transform.hpp"
   ...etc...
   ```
   This matches what everyone else does and what you need, if you download the full github repository instead of the source code light zip.
4. Create an environment variable **glm_INCLUDE_DIRS** and point it to the **first** ../glm/.. of the glm library location.
   <div style="text-align: center">
      <img src="./png/glfw-install-2.png" style="width: 20%;">
      <img src="./png/glm-install-3.png" style="width: 30%;">
      <figcaption></figcaption>
      <br>
   </div>
5. Maybe reboot the system or not (as usual in Windows)
6. The CmakeLists.txt configuration should now set the correct paths for compilation. Test by **deleting CMAKE cache and reconfigure**.

#### Download Pybind11
This is **only necessary** to generate a **Python** export. (Source: https://pybind11.readthedocs.io/en/stable/installing.html). Make sure to also install the pybind-stup generator as outlined in the setup script.
1. run
   ```
   $> pip install pybind11
   ```

# How to build for C++
* Make sure all dependencies are installed as outlined in the previous section
* Open the project with VSCode and run CMake config
* Check out sample_viewer.cpp for a guideline on how to use the viewer inside a program

# How to build for Python
* Open a terminal from inside the project folder
* Check out setup_vkviewer.py
* The Python bindings script is a bit static, so some paths may have to be adjusted
   <div style="text-align: center">
      <img src="./png/python-setup.png" style="width: 50%;">
      <figcaption></figcaption>
      <br>
   </div>
* follow the instructions in the comment section of setup_vkviewer.py at the top
