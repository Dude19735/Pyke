cmake_minimum_required (VERSION 3.22)

file(GLOB SOURCES
    ./toojpeg-master/*.h
    ./toojpeg-master/*.cpp
)

add_library(TooJpeg ${SOURCES})