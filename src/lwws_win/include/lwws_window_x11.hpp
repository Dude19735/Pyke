#pragma once
#include <stdexcept>
#include <condition_variable>
#include "lwws_window.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <cstring>

namespace LWWS {
    // class RedrawQueue {
    //     std::queue<int> _queue;
    //     std::mutex _mutex;
    // public:
    //     RedrawQueue(){}
    //     void pushRedrawEvent(){
    //         auto lock = std::lock_guard<std::mutex>(_mutex);
    //         _queue.push(1);
    //     }

    //     bool hasRedrawEvent(){
    //         auto lock = std::lock_guard<std::mutex>(_mutex);
    //         if(_queue.size() == 0) return false;
    //         _queue.pop();
    //         return true;
    //     }
        
    // };

    class LWWS_Window_X11: public LWWS_Window{
        Display* _display;
        Window _window;
        Screen* _screen;
        int _screenId;
        XEvent _ev;
        GC _gc;

        long unsigned int _cFocused;
        long unsigned int _cMaxhorz;
        long unsigned int _cMaxvert;
        long unsigned int _cHidden;
        int _winstate = 0; // 0 = normal, 1 = maximized, 2 = minimized

        KeySym _key;
        char _text[255];

    public:
        LWWS_Window_X11(
            std::string windowTitle,
            int width,
            int height,
            bool resizable,
            bool disableMousePointerOnHover=false,
            int hoverTimeoutMS=500,
            bool bindSamples=false
        ) 
        : 
        LWWS_Window(width, height, disableMousePointerOnHover, hoverTimeoutMS, bindSamples),
        _display(nullptr),
        _window(0),
        _screen(nullptr),
        _screenId(0),
        _ev({}),
        _cFocused(0),
        _cMaxhorz(0),
        _cMaxvert(0),
        _cHidden(0),
        _winstate(0),
        _key(0)
        {
            unsigned long white;

            /* use the information from the environment variable DISPLAY 
            to create the X connection:
            */	
            _display = XOpenDisplay((char *)0);
            _screenId = DefaultScreen(_display);
            white = WhitePixel(_display,_screenId);  /* get color white */

            _window = XCreateSimpleWindow(_display,DefaultRootWindow(_display),0,0,	_windowWidth, _windowHeight, 5, white, white);
            XSetStandardProperties(_display, _window, windowTitle.c_str(), "", None, NULL, 0, NULL);

            Atom wm_delete = XInternAtom(_display, "WM_DELETE_WINDOW", 1);	
            XSetWMProtocols(_display, _window, &wm_delete, 1 );

            _cFocused = XInternAtom(_display, "_NET_WM_STATE_FOCUSED", 1);
            _cMaxhorz = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", 1);
            _cMaxvert = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", 1);
            _cHidden =  XInternAtom(_display, "_NET_WM_STATE_HIDDEN", 1);

            XSelectInput(_display, _window, 
                ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                PointerMotionMask | FocusChangeMask | StructureNotifyMask | PropertyChangeMask
            );

            // create the Graphics Context
            _gc=XCreateGC(_display, _window, 0,0);

            if(!resizable){
                auto sh = XAllocSizeHints();
                sh->flags = PMinSize | PMaxSize;
                sh->min_width = sh->max_width = 100;
                sh->min_height = sh->max_height = 100;
                //XSetWMNormalHints(d, w, sh);
                XSetWMSizeHints(_display, _window, sh, XA_WM_NORMAL_HINTS);
                XFree(sh);
            }
        }

        ~LWWS_Window_X11(){
            XFreeGC(_display, _gc);
            XDestroyWindow(_display, _window);
	        XCloseDisplay(_display);
        }

        void getX11XcbWindowDescriptors(Display*& display, uint32_t& screenId){
            display = _display;
            screenId = static_cast<uint32_t>(_screenId);
        }

        void getX11XlibWindowDescriptors(Display*& display, Window& window){
            display = _display;
            window = _window;
        }

        inline void windowEvents_Init() {
            XClearWindow(_display, _window);
	        XMapRaised(_display, _window);
        }

        inline bool windowEvents_Exist() {
            if(_windowShouldClose) return false;
            return true;
        }

        inline void windowEvents_Pump(){
            // std::unique_lock<std::mutex> lock(_mutex);
            // _condition.wait(lock, [this]() {
            //     // continue if we have pending events
            //     return XPending(_display)>0 || _redraw;
            // });
            // if(_redraw) {
            //     queueRedraw();
            //     _redraw = false;
            // }

            windowEventPump();
        }

        inline void emit_windowEvent_Paint(){
            // {
            //     std::unique_lock<std::mutex> lock(_mutex);
            //     _redraw = true;
            // }
            // _condition.notify_one();
            // _redrawQueue.pushRedrawEvent();
            // Leave this for reference. It is NOT thread safe, though
            // =======================================================
            // auto lock = std::lock_guard<std::mutex>(_mutex);

            XExposeEvent pEv;
            memset(&pEv, 0, sizeof(pEv));
            pEv = {
                .type = Expose,
                .serial = 0,
                .send_event = false,
                .display = _display,
                .window = _window,
                .x = _windowPosX,
                .y = _windowPosY,
                .width = _windowWidth,
                .height = _windowHeight,
                .count = 0
            };

            XSendEvent(_display, _window, true, ExposureMask, reinterpret_cast<XEvent*>(&pEv));
            XFlush(_display);
            // std::cout << "send paint event " << ret << std::endl;
        }

        bool frameSize(int& width, int& height){
            Window root;
            int x, y;
            unsigned int w, h, border_width, depth;
            int status = XGetGeometry(_display, _window, &root, &x, &y, &w, &h, &border_width, &depth);
            if(status == 0) return false;
            width = static_cast<int>(w);
            height = static_cast<int>(h);
            return true;
        }

    private:
        void windowEventPump(){
            // if(_redrawQueue.hasRedrawEvent()) wndPaint(this);
            XNextEvent(_display, &_ev);
            switch(_ev.type){
                case Expose:{
                    // std::cout << "paint event" << std::endl;
                    if(_ev.xexpose.count==0) {
                        // std::cout << "..... paint event" << std::endl;
                        wndPaint(this);
                    }
                    break;
                } 
                case ClientMessage:{
                    std::string xx = XGetAtomName(_display, _ev.xclient.message_type);
                    if (xx.compare("WM_PROTOCOLS") == 0) {
                        wndCloseOperations(this);
                    }
                    break;
                }
                case KeyPress:{
                    unsigned int kk = _ev.xkey.keycode;
                    if(LWWS_Key_X11::KeyFilter(kk)) break;

                    auto special = LWWS_Key_X11::ToSpecialKey(kk);
                    if(special != LWWS_Key::Special::RandomKey){
                        wndSpecialKeyPressed(this, special, ButtonOp::Down, {});
                        break;
                    }

                    // _text[0] can be a char that corresponds to what the user pressed
                    // but it can also be some control sequence like dec(19) in the case of ctrl+s.
                    // _key seems to always be the underlying char => use that one
                    _ev.xkey.keycode = kk;
                    if(XLookupString(&_ev.xkey, _text, 255, &_key, 0)==1) {
                        wndCharPressed(this, static_cast<char>(_key) /*_text[0]*/, ButtonOp::Down);
                        break;
                    }

                    // some unsupported key pressed
                    break;
                }
                case KeyRelease:{
                    unsigned int kk = _ev.xkey.keycode;
                    if(LWWS_Key_X11::KeyFilter(kk)) break;

                    auto special = LWWS_Key_X11::ToSpecialKey(kk);
                    if(special != LWWS_Key::Special::RandomKey){
                        wndSpecialKeyPressed(this, special, ButtonOp::Up, {});
                        break;
                    }

                    if(XLookupString(&_ev.xkey, _text, 255, &_key,0)){
                        XEvent ev;
                        if(XPending(_display) && XPeekEvent(_display, &ev)){
                            if(ev.type == KeyPress && ev.xkey.keycode == kk) break;
                        }

                        // _text[0] can be a char that corresponds to what the user pressed
                        // but it can also be some control sequence like dec(19) in the case of ctrl+s.
                        // _key seems to always be the underlying char => use that one
                        wndCharPressed(this, _key /*_text[0]*/, ButtonOp::Up);
                        break;
                    }

                    // some unsupported key pressed
                    break;
                }
                case ConfigureNotify:{
                    auto x = _ev.xconfigure.x;
                    auto y = _ev.xconfigure.y;
                    auto width = _ev.xconfigure.width;
                    auto height = _ev.xconfigure.height;
                    if(x != _windowPosX || y != _windowPosY) wndMoved(this, x, y);
                    if(width != _windowWidth || height != _windowHeight) wndResize(this, width, height, false);
                    break;
                }
                case ButtonPress:{
                    if(_ev.xbutton.button == 1) wndMousePressed(this, MouseButton::Left, ButtonOp::Down);
                    else if(_ev.xbutton.button == 2) wndMousePressed(this, MouseButton::Middle, ButtonOp::Down);
                    else if(_ev.xbutton.button == 3) wndMousePressed(this, MouseButton::Right, ButtonOp::Down);
                    else if(_ev.xbutton.button == 4) wndMouseScroll(this, 1.0); // scroll up
                    else if(_ev.xbutton.button == 5) wndMouseScroll(this, -1.0); // scroll down
                    break;
                }
                case ButtonRelease:{
                    if(_ev.xbutton.button == 1) wndMousePressed(this, MouseButton::Left, ButtonOp::Up);
                    else if(_ev.xbutton.button == 2) wndMousePressed(this, MouseButton::Middle, ButtonOp::Up);
                    else if(_ev.xbutton.button == 3) wndMousePressed(this, MouseButton::Right, ButtonOp::Up);
                    /**
                     * ignore these two ON PURPOSE => no need for this
                     * else if(_ev.xbutton.button == 4) wndMouseScroll(this, 120); // scroll up
                     * else if(_ev.xbutton.button == 5) wndMouseScroll(this, -120); // scroll down
                     */
                    break;
                }
                case MotionNotify:{
                    wndMouseMoved(this, _ev.xbutton.x, _ev.xbutton.y);
                    break;
                }
                case FocusIn:{
                    wndSetActive(this, true);
                    break;
                }
                case FocusOut:{
                    wndSetActive(this, false);
                    break;
                }
                case PropertyNotify:{
                    /**
                     * Needs to be after focus in and out
                     */
                    auto event = propertyNotifyEvent();
                    int width = 0;
                    int height = 0;
                    if(!frameSize(width, height)){
                        width = _windowWidth;
                        height = _windowHeight;
                    }

                    if(event == WindowAction::Minimized) wndResize(this, width, height, true);
                    // the other return value here is 'Maximized'
                    else if(event == WindowAction::Maximized) wndResize(this, width, height, false);

                    break;
                }
            }
        }

        WindowAction propertyNotifyEvent(){
            /**
             * Source: https://stackoverflow.com/questions/69249370/how-do-i-capture-minimize-and-maximize-events-in-xwindows
             * Thank You Scott Franco!
             */
            std::string xx = XGetAtomName(_display, _ev.xproperty.atom);
            if(xx.compare("_NET_WM_STATE")==0){
                unsigned long after = 1L;
                int focused = 0;
                int maxhorz = 0;
                int maxvert = 0;
                int hidden = 0;
                
                do{
                    Atom type;		/* actual_type_return */
                    int format;		/* actual_format_return */
                    unsigned long length;	/* nitems_return */
                    unsigned char* dp;	/* prop_return */
                    int status = XGetWindowProperty(_display, _window, _ev.xproperty.atom,
                                                    0L, after, 0,
                                                    4/*XA_ATOM*/, 
                                                    &type, &format, &length, &after, &dp);
                                                
                    if (status == Success && type == 4/*XA_ATOM*/ && dp && format == 32 && length) {

                        for (unsigned int i = 0; i < length; i++) {

                            auto prop = ((Atom*)dp)[i];

                            if (prop == _cFocused) focused = 1;
                            if (prop == _cMaxhorz) maxhorz = 1;
                            if (prop == _cMaxvert) maxvert = 1;
                            if (prop == _cHidden) hidden = 1;

                        }

                    }
                } while(after);
                int lws = 0;
                if (hidden) {
                    lws = _winstate;
                    _winstate = 2;
                    if (lws != _winstate) return WindowAction::Minimized;
                } 
                else if (maxhorz || maxvert) {
                    lws = _winstate;
                    _winstate = 1;
                    if (lws != _winstate) return WindowAction::Maximized;
                } 
                else if (focused) {
                    lws = _winstate;
                    _winstate = 0;
                    // this one should be a window, but Windows API doesn't really support that
                    if (lws != _winstate) return WindowAction::Maximized;

                }
            }

            return WindowAction::NoAction;
        }
    };
}