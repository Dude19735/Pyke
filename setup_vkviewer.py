#!/usr/bin/env python3
# encoding: utf-8

# Linux ####################################################################################################
# This part is to install the viewer properly without needing sudo permission: create virtual environment
# in local home
#
# To create a virtual environment, do:
# $> sudo apt install python3-virtualenv
# $> mkdir /home/lol/.venvs
# $> virtualenv /home/lol/.venvs/standard
#
# Activate virtual environment:
# $> source /home/lol/.venvs/standard/bin/activate
# If the setup is not run inside a user environment, it will complain about lacking permission at the
# end of the setup
#
# Install numpy
# $> pip install numpy
#
# Install pybind11-stubgen
# $> pip install pybind11-stubgen
#
# Run installation
# cd to where setup.py is located
# $> clear && python3 setup.py build
############################################################################################################

# Windows ##################################################################################################
# Install setuptools
# cmd$> pip install setuptools
#
# Install numpy
# $> pip install numpy
#
# Install pybind11-stubgen
# cmd$> pip install pybind11-stubgen
#
# Run installation
# cd to where setup.py is located
# $> clear; python3 setup.py build
############################################################################################################

import sysconfig
import os
import sys
import shutil
import subprocess
import platform
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import pybind11_stubgen
# from distutils.core import setup, Extension

# from src.vk_renderer4.objects.compile_glsl import ToSPV

class PyInit:
    def __init__(self, module_name):
        p = os.path.split(os.path.realpath(__file__))[0]
        path = p + "/" + module_name + "/__init__.py"

        stubs = [
"""
import os
""",
"""
try:
    from _vkviewer import *
except ImportError:
    from ._vkviewer import *

""",
        ]

        pp = os.path.split(path)[0]
        if not os.path.exists(pp):
            os.mkdir(pp)

        with open(path, "w") as f:
            f.writelines([''] + stubs + [''])

class StubsLinux:
    def __init__(self):
        # p = os.path.split(os.path.realpath(__file__))[0]
        # subprocess.run([p + "/stubs.sh"], shell=True)
        sys.argv = sys.argv[:1] + [
            "--enum-class-locations", "vk_render_type:pyke",
            "--enum-class-locations", "vk_topology:pyke",
            "--enum-class-locations", "vk_cull_mode:pyke",
            "--enum-class-locations", "vk_buffer_characteristics:pyke",
            "pyke"
        ]
        pybind11_stubgen.main()

class StubsWindows:
    def __init__(self):
        # p = os.path.split(os.path.realpath(__file__))[0]
        sys.argv = sys.argv[:1] + [
            "--enum-class-locations", "vk_render_type:pyke",
            "--enum-class-locations", "vk_topology:pyke",
            "--enum-class-locations", "vk_cull_mode:pyke",
            "--enum-class-locations", "vk_buffer_characteristics:pyke",
            "pyke"
        ]
        pybind11_stubgen.main()
        # subprocess.run([p + "/stubs.sh"], shell=True)

class ConfigLinux:
    def __init__(self):
        cDir = os.getcwd()
        self.install_path = "/usr/lib/python3.11"
        for p in sys.path:
            if p.find(".venvs/standard") >= 0:
                self.install_path = p
                break

        # config compiler
        self.extra_compile_args = []
        sys_compile_args = sysconfig.get_config_var('CFLAGS')
        if sys_compile_args:
            self.extra_compile_args += sys_compile_args.split()

        # self.extra_compile_args = ["-fsanitize=address"] + self.extra_compile_args + ["-std=gnu++20"]
        self.extra_compile_args = self.extra_compile_args + ["-std=gnu++20"]

        self.include_dirs = [
            cDir,
            "/usr/include",
            "/usr/include/python3.11"
        ]
        self.library_dirs = [
            "/usr/lib/x86_64-linux-gnu"
        ]

        # NOTE: linking -lstdc++ is important for all std lib c++ things
        # Symptoms for not doing so are:
        # Python exception 'undefined symbol: _ZTVN10__cxxabiv117__class_type_infoE'
        # when trying to import the module
        #
        # In Linux all libraries are passed without 'lib' and '.so'
        # self.libraries = ["asan", "vulkan", "stdc++", "SM", "ICE", "X11", "Xext", "xcb"]
        self.libraries = ["vulkan", "stdc++", "SM", "ICE", "X11", "Xext", "xcb"]

class ConfigWindows:
    def __init__(self):
        python = "C:/Python311/"
        cDir = os.getcwd()
        self.install_path = python + "Lib/site-packages"

        self.extra_compile_args = []
        sys_compile_args = sysconfig.get_config_var('CFLAGS')
        if sys_compile_args:
            self.extra_compile_args += sys_compile_args.split()
        self.extra_compile_args += ["/std:c++latest"]

        # NOTE: the sequence of the include directories matters...
        self.include_dirs = [
            python + "include",
            # self.install_path + "/pybind11/include",
            "C:/pybind11-master/include",
            cDir,
            os.environ["VULKAN_SDK"] + "/include",
            os.environ["glm_INCLUDE_DIRS"],
            os.environ["Boost_INCLUDE_DIR"]
        ]
        self.library_dirs = [
            os.environ["VULKAN_SDK"] + "/Lib"
        ]

        # NOTE: the path for User32, Gdi32 and Shell32 doesn't need to be specified
        # It's just magically found
        self.libraries = ["vulkan-1", "User32", "Gdi32", "Shell32"]

# safeguard... ###########################################################
if not __name__ == "__main__":
    exit(0)
##########################################################################

# check Python version
version = sys.version.split(".")
if version[0] != '3':
    print("Unsupported Python version: only {0}.XX supported", 3)
    exit(-1)
if int(version[1]) > 11:
    print("Python 3.12 not yet suported!")
    exit(-1)

module_name = "pyke"
script_path = os.path.split(os.path.realpath(__file__))[0]

# run spv compiler first
# ToSPV(module_name).compile()
PyInit(module_name)

if platform.system() == "Linux":
    install_config = ConfigLinux()
elif platform.system() == "Windows":
    install_config = ConfigWindows()
else:
    print("Unsupported platform!")
    exit(-1)

# is_debug = False
# with_opencv = True

# if not is_debug:
#     # Release mode
#     extra_compile_args += ["-rdynamic", "-std=gnu++20"]
# else:
#     # Debug mode
#     extra_compile_args += ["-rdynamic", "-std=gnu++20"]

build_run = "build" in sys.argv
if build_run:
    s = setup(
        name="_vkviewer",
        ext_modules=[
            Extension(\
                "_vkviewer",\
                sources=[script_path + "/src/pyexp_vkviewer.cpp"],\
                include_dirs=install_config.include_dirs,\
                library_dirs=install_config.library_dirs,\
                libraries=install_config.libraries,\
                extra_compile_args=install_config.extra_compile_args,\
                language='c++20',
                define_macros=[
                    ("PYVK", '1'), 
                    # ("_DEBUG", '1')
                ]
            )
        ],
        zip_safe=False,
        cmdclass={"build_ext": build_ext}
    )

for obj in os.walk("./build"):
    build_folder = list(filter(lambda f: f.find("lib.") >= 0, obj[1]))
    if build_folder:
        path = obj[0] + "/" + build_folder[0]
        if platform.system() == "Linux":
            files = list(filter(lambda f: f.find(".so") >= 0, os.listdir(path)))
        elif platform.system() == "Windows":
            files = list(filter(lambda f: f.find(".pyd") >= 0, os.listdir(path)))
        else:
            print("Unsupported platform!")
            exit(0)
        if len(files) >= 1:
            for file in files:
                shutil.move(path + "/" + file, script_path + "/" + module_name + "/" + file)
                # shutil.copy(path + "/" + file, script_path + "/" + module_name)
                print("Move " + path + "/" + file + " to " + script_path + "/" + module_name + "/" + file)
        else:
            print("Nothing to move...")

# # move to python search path
# # if installation already exists, remove it
mod_path = install_config.install_path + "/" + module_name
if os.path.exists(mod_path):
    print("Removing old binary version")
    shutil.rmtree(mod_path)

# if build_run and os.path.exists(script_path + "/" + module_name):
#     print("Removing old python export")
#     shutil.rmtree(script_path + "/" + module_name)

print("Moving Python module to " + install_config.install_path)
shutil.move(script_path + "/" + module_name + "/", install_config.install_path)

# print("Copy Python module to " + install_config.install_path)
# shutil.copytree(script_path + "/" + module_name + "/", install_config.install_path + "/" + module_name)

if platform.system() == "Linux":
    StubsLinux()
elif platform.system() == "Windows":
    StubsWindows()
else:
    print("Unsupported platform!")
    exit(-1)

# print("Copy Python interface to " + install_path)
# shutil.copy(script_path + "/stubs/" + module_name + "/__init__.pyi", script_path + "/" + module_name + "/__init__.pyi")
# shutil.copy(script_path + "/stubs/" + module_name + "/_vkviewer.pyi", script_path + "/" + module_name + "/_vkviewer.pyi")

print("Move Python interface to " + install_config.install_path)
shutil.move(script_path + "/stubs/" + module_name + "/__init__.pyi", install_config.install_path + "/" + module_name + "/__init__.pyi")
shutil.move(script_path + "/stubs/" + module_name + "/_vkviewer.pyi", install_config.install_path + "/" + module_name + "/_vkviewer.pyi")

if os.path.exists("./stubs"):
    print("clean up")
    shutil.rmtree("./stubs")
