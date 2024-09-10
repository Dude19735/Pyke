#pragma once

#define NOMINMAX
#include <cmath>
#include <format>

namespace LWWS {
    typedef int TViewportId;

    class LWWS_Viewport {
        float _posW;
        float _posH;
        float _width;
        float _height;
        float _originalPosW;
        float _originalPosH;
        float _originalW;
        float _originalH;
        float _originalParentW;
        float _originalParentH;
        int _borderWidth;
        std::string _borderColor;
        std::string _bgColor;
        TViewportId _viewportId;
    public:
        LWWS_Viewport(TViewportId viewportId, int posw, int posh, int width, int height, int borderWidth=1, const std::string& borderColor="#000000", const std::string& bgColor="#FFFFFFFF")
        : 
        _posW(static_cast<float>(posw)), 
        _posH(static_cast<float>(posh)), 
        _width(static_cast<float>(width-2*borderWidth)), 
        _height(static_cast<float>(height-2*borderWidth)), 
        _originalPosW(static_cast<float>(posw)), 
        _originalPosH(static_cast<float>(posh)),
        _originalW(static_cast<float>(width-2*borderWidth)), 
        _originalH(static_cast<float>(height-2*borderWidth)),
        _originalParentW(-1.0f),
        _originalParentH(-1.0f),
        _borderWidth(borderWidth),
        _borderColor(borderColor),
        _bgColor(bgColor),
        _viewportId(viewportId)
        {
            testRGBColor("borderColor", borderColor);
            testRGBColor("bgColor", bgColor);
        }

        ~LWWS_Viewport(){}

        void resize(int parentW, int parentH){
            if(_originalParentH < 0 || _originalParentW < 0){
                throw std::runtime_error("Set parent size for viewport before resize!");
            }

            float nw = static_cast<float>(parentW);
            float nh = static_cast<float>(parentH);

            float posW = _originalPosW / _originalParentW * nw;
            float posH = _originalPosH / _originalParentH * nh;
            float width = _originalW / _originalParentW * nw;
            float height = _originalH / _originalParentH * nh;

            _posW = std::roundf(posW);
            _posH = std::roundf(posH);
            _width = std::roundf(width);
            _height = std::roundf(height);

            // std::cout << _width << " " << parentW << std::endl;
        }

        void setParentSize(int parentW, int parentH){
            _originalParentH = parentH;
            _originalParentW = parentW;
        }

        int posW() const { return static_cast<int>(_posW); }
        int posH() const { return static_cast<int>(_posH); }
        int width() const { return static_cast<int>(_width); }
        int height() const { return static_cast<int>(_height); }
        int borderWidth() const { return _borderWidth; }
        const std::string& borderColor() const { return _borderColor; }
        const std::string& bgColor() const { return _bgColor; }

    private:
        void testRGBColor(const std::string& name, const std::string& color){
            const std::set<char> p = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
            std::string errmsg = "{0} must have format '#RRGGBB' where each character is between 0 and F (in hex), but is {1}!";
            if(color.size() != 7 || color.at(0) != '#' || !std::all_of(color.begin()+1, color.end(), [&](char x){ return p.contains(x); })){
                throw std::runtime_error(std::vformat(errmsg, std::make_format_args(name, color)));
            }
        }
    };
}