#pragma once

#include "../Defines.h"
#include "../Vk_Viewer.hpp"

namespace VK4 {
    struct I_LayoutPack {
        Vk_CameraSpecs specs;
        Vk_RGBColor clearColor;
    };

    class I_Layout {
    public:
        virtual std::vector<Vk_CameraInit> vk_layoutList(int width, int height) const = 0;
        virtual const int vk_count() const = 0;
    };
}