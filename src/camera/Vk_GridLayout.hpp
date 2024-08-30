#pragma once

#include <vector>
#include <set>

#include "../Defines.h"
#include "I_Layout.hpp"

namespace VK4 {
    class Vk_GridLayout: public I_Layout {
    public:
        Vk_GridLayout(int xCount, int yCount, int xSpacing, int ySpacing) 
        : 
        _yCount(yCount), _xCount(xCount), _xSpacing(xSpacing), _ySpacing(ySpacing) 
        {
            if(yCount < 1 || xCount < 1){
                Vk_Logger::RuntimeError(typeid(this), "Both yCount and xCount must be greater than 0 but yCount={0} and xCount={1}!", yCount, xCount);
            }

            if(ySpacing < 0 || xSpacing < 0){
                Vk_Logger::RuntimeError(typeid(this), "Spacing must be greater or equal!");
            }
        }

#ifndef PYVK
        Vk_GridLayout(int xCount, int yCount, int xSpacing, int ySpacing, const std::vector<I_LayoutPack>& pack) 
        : 
        _yCount(yCount), _xCount(xCount), _xSpacing(xSpacing), _ySpacing(ySpacing) 
        {
            if(yCount < 1 || xCount < 1){
                Vk_Logger::RuntimeError(typeid(this), "Both yCount and xCount must be greater than 0!");
            }

            if(ySpacing < 0 || xSpacing < 0){
                Vk_Logger::RuntimeError(typeid(this), "Spacing must be greater or equal 0!");
            }

            if(pack.size() > yCount*xCount){
                Vk_Logger::RuntimeError(typeid(this), "If specs are passed in constructor, at most yCount*xCount are allowed!");
            }

            vk_addCamera(0, 0, pack, false);

            if(cameras.size() != pack.size()){
                Vk_Logger::RuntimeError(typeid(this), "Something went wrong in grid layout creation...");
            }
        }
#endif

        std::vector<Vk_CameraInit> vk_layoutList(int width, int height) const {
            // auto surface = viewer.vk_surface();
            // auto extent = viewer.vk_device()->vk_swapchainSupportActiveDevice(surface).capabilities.currentExtent;
            uint32_t viewportWidth = static_cast<uint32_t>(std::floor(static_cast<float>(width - 2*_xCount*_xSpacing) / static_cast<float>(_xCount)));
            uint32_t viewportHeight = static_cast<uint32_t>(std::floor(static_cast<float>(height - 2*_yCount*_ySpacing) / static_cast<float>(_yCount)));

            std::vector<Vk_CameraInit> res;
            int camId = 0;
            for(const auto& c : cameras){
                res.push_back({
                    VK4::Vk_CameraInit{
                        .camId = camId,
                        .gridX = c.x,
                        .gridY = c.y,
                        .viewport = VK4::Vk_Viewport{
                            .x = static_cast<int32_t>((c.x*(viewportWidth + 2*_xSpacing) + _xSpacing)),
                            .y = static_cast<int32_t>((c.y*(viewportHeight + 2*_ySpacing) + _ySpacing)),
                            .width = viewportWidth,
                            .height = viewportHeight,
                            .clearColor = c.lp.clearColor
                        },
                        .specs = c.lp.specs
                    }
                });
                camId++;
            }

            return res;
        }

        const int vk_count() const { return static_cast<int>(cameras.size()); }

        /**
         * Add camera specs at location (v,h) where v is the vertical and h the horizontal coordinate
         * starting at zero. If override=true, a camera at a specific location
         * will be overridden. Indexes start at 0. x coord runs over the width, y coord runs over
         * the height.
         * 
         * @returns the number of inserted elements. That is 1 if the operation could be completed. 
         *          If override=false and (x,y) is already taken, the return value will be 0. If override=true and (x,y) 
         *          is already taken and (x,y) fit into the grid, then the return value will be 1.
        */
        int vk_addCamera(int x, int y, const I_LayoutPack& pack, bool override=false){
            if(y < 0 || y >= _yCount) {
                Vk_Logger::Error(typeid(this), "CameraId with position x={0}, y={1} doesn't exist. Possible are x=0 to x={2} and y=0 to y={3}", x, y, _xCount, _yCount);
                return 0;
            }
            if(x < 0 || x >= _xCount) {
                Vk_Logger::Error(typeid(this), "CameraId with position x={0}, y={1} doesn't exist. Possible are x=0 to x={2} and y=0 to y={3}", x, y, _xCount, _yCount);
                return 0;
            }

            elem e {
                .y=y,
                .x=x,
                .lp=pack
            };

            auto loc = cameras.find(e);
            if(override && loc != cameras.end()){
                // remove fist, then insert
                cameras.erase(loc);
                cameras.insert(e);
                return 1;
            }
            
            if(!override && loc != cameras.end()){
                return 0;
            }

            cameras.insert(e);
            return 1;
        }

#ifndef PYVK
        /**
         * Add all cameras inside specs starting at (startV, startH) where startV is the vertical and
         * startH the horizontal starting coordinate, both starting from zero. If override=true, a camera
         * already present at a location will be overridden. If override=false, only empty spots will
         * be filled. Indexes start at 0. x coord runs over the width, y coord runs over
         * the height.
         * 
         * @returns the number of inserted elements. That is specs.size() if all operations successful. 
         *          If override=false, this will skip locations that already have an entry and the method will return a value 
         *          smaller than specs.size() if not all cameras inside specs can be inserted. If override=true, the return 
         *          value will be greater zero but at most specs.size() as long as startX and startY fit into the grid.
        */
        int vk_addCamera(int startX, int startY, const std::vector<I_LayoutPack>& pack, bool override=false){
            if(startX < 0 || startX >= _xCount) return 0;
            if(startY < 0 || startY >= _yCount) return 0;

            int index=0;
            int sx = startX;
            for(int y=startY; y<_yCount; ++y){
                for(int x=sx; x<_xCount; ++x){
                    elem e {
                        .y=y,
                        .x=x,
                        .lp=pack.at(index)
                    };
                    auto loc = cameras.find(e);
                    if(override && loc != cameras.end()){
                        cameras.erase(loc);
                        cameras.insert(e);
                        index++;
                    }
                    else if(!override && loc != cameras.end()){
                        // skip one
                        continue;
                    }
                    else {
                        cameras.insert(e);
                        index++;
                    }

                    if(index==pack.size()){
                        return index;
                    }
                }
                sx = 0;
            }

            return index;
        }
#endif

    private:
        struct elem {
            int y, x;
            I_LayoutPack lp;

            bool const operator==(const elem &other) const {
                return x == other.x && y == other.y;
            }

            bool const operator<(const elem &other) const {
                return x < other.x || (x == other.x && y < other.y);
            }
        };

        int _yCount;
        int _xCount;
        int _xSpacing;
        int _ySpacing;
        std::set<elem> cameras;
    };
}