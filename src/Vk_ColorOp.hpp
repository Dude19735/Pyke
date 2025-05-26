#pragma once

#include "Vk_Coloring.hpp"
#include "Vk_Logger.hpp"

namespace VK4{
    class Vk_ColorOp {
        // TODO: synchronize all operations for RGB and RGB colors
    public:
        static Vk_OklabColor rgb_to_oklab(const Vk_RGBColor& rgb)
        {
            float l = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
            float m = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
            float s = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;

            float l_ = cbrtf(l);
            float m_ = cbrtf(m);
            float s_ = cbrtf(s);

            Vk_OklabColor res;
            res.L = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
            res.a = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
            res.b = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;

            return res;
        }

        // source: https://bottosson.github.io/posts/oklab/
        static Vk_RGBColor oklab_to_rgb(const Vk_OklabColor& oklab)
        {
            float l_ = oklab.L + 0.3963377774f * oklab.a + 0.2158037573f * oklab.b;
            float m_ = oklab.L - 0.1055613458f * oklab.a - 0.0638541728f * oklab.b;
            float s_ = oklab.L - 0.0894841775f * oklab.a - 1.2914855480f * oklab.b;

            float l = l_ * l_ * l_;
            float m = m_ * m_ * m_;
            float s = s_ * s_ * s_;

            Vk_RGBColor res;
            res.r = std::max(std::min(4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s, 1.0f), 0.0f);
            res.g = std::max(std::min(-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s, 1.0f), 0.0f);
            res.b = std::max(std::min(-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s, 1.0f), 0.0f);

            return res;
        }

        static Vk_OklabColor oklab_lerp(float p, Vk_OklabColor& from, Vk_OklabColor& to) {
            Vk_OklabColor res;
            res.L = (1.0f - p) * from.L + p * to.L;
            res.a = (1.0f - p) * from.a + p * to.a;
            res.b = (1.0f - p) * from.b + p * to.b;

            return res;
        }

        static Vk_RGBColor rgb_lerp(float p, const Vk_RGBColor& from, const Vk_RGBColor& to) {
            auto oklab_from = rgb_to_oklab(from);
            auto oklab_to = rgb_to_oklab(to);
            auto oklab_res = oklab_lerp(p, oklab_from, oklab_to);
            auto rgb_res = oklab_to_rgb(oklab_res);

            return rgb_res;
        }

        static std::vector<Vk_RGBColor> rgb_vector_lerp(const std::vector<float>& p, const Vk_RGBColor& from, const Vk_RGBColor& to) {
            std::vector<Vk_RGBColor> res;
            res.reserve(p.size());
            for(auto pp : p){
                if(pp < 0.0f) {
                    pp = 0.0f;
                    Vk_Logger::Warn(typeid(NoneObj), "Lerp value {0} smaller than 0", pp);
                }
                else if(pp > 1.0f) {
                    pp = 1.0f;
                    Vk_Logger::Warn(typeid(NoneObj), "Lerp value {0} greater than 1.0", pp);
                }
                res.push_back(rgb_lerp(pp, from, to));
            }

            return res;
        }

        static void rgb_target_vector_lerp(const std::vector<float>& p, const Vk_RGBColor& from, const Vk_RGBColor& to, std::vector<Vk_RGBColor>& target) {
            for(auto pp : p){
                if(pp < 0.0f) {
                    pp = 0.0f;
                    Vk_Logger::Warn(typeid(NoneObj), "Lerp value {0} smaller than 0", pp);
                }
                else if(pp > 1.0f) {
                    pp = 1.0f;
                    Vk_Logger::Warn(typeid(NoneObj), "Lerp value {0} greater than 1.0", pp);
                }
                target.push_back(rgb_lerp(pp, from, to));
            }
        }
    };
}
