#pragma once

#if defined(_WIN32) || defined(_WIN64)
   
#   include <windows.h>

#endif // defined(_WIN32) || defined(_WIN64)


#ifndef _GD_WIN_BEGIN
#define _GD_WIN_BEGIN namespace gd_win {
#define _GD_WIN_END }
#endif

_GD_WIN_BEGIN

namespace os
{

    class window
{

public:
   window() = default;
   virtual ~window() = default;

   window(const window&) = delete;
   window& operator=(const window&) = delete;

   window(window&&) = delete;
   window& operator=(window&&) = delete;

public: // @API [tag: attributes] [description: Get native window handle.] [jump: handle__] 
   
};

} // namespace os

#if defined(_WIN32) || defined(_WIN64)

class window_main
{
public:
    window_main() = default;
    virtual ~window_main() = default;
    
    window_main(const window_main&) = delete;
    window_main& operator=(const window_main&) = delete;
    
    window_main(window_main&&) = delete;
    window_main& operator=(window_main&&) = delete;


};

#endif // defined(_WIN32) || defined(_WIN64)


_GD_WIN_END