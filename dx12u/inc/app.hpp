
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


/**
* @file      app.hpp
* @brief     dx12 utilities app helpers
*/

#ifndef DX12U_APP_HPP
#define DX12U_APP_HPP

#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <functional>

namespace dx12u
{
    /**
    * @brief mouse button state
    */
    enum class mouse_button
    {
        none,
        up,
        down
    };

    /**
    * @brief get the app window width
    * @return app window width
    */
    std::size_t get_window_width() noexcept;

    /**
    * @brief get the app window height
    * @return app window height
    */
    std::size_t get_window_height() noexcept;

    /**
    * @brief get the app virtual key state
    * @return true if vk is pressed; false otherwise
    */
    bool is_key_pressed(std::uint8_t vk) noexcept;

    /**
    * @brief get the app window handle
    * @return hwnd
    */
    HWND get_hwnd() noexcept;

    /**
    * @brief resize the window
    * @param w width of the window
    * @param h height of the window
    */
    void resize_window(std::size_t w, std::size_t h);

    /**
    * @brief request termination of the application
    */
    void quit_app();

    /**
    * @brief enable capture of mouse events when the cursor is outside the window
    * @param capture true enables the capture, false stops the capture
    * @note this also hides the mouse because it doesn't display correctly in capture mode
    */
    void set_mouse_capture(bool capture);

    /**
    * @brief create and process an app window
    * @param name window name
    * @param w width of the window
    * @param h height of the window
    * @param init initialization function called after window creation or window resize
    * @param step loop step function called every frame
    * @param release function called at exit time or at window resize
	* @param process_keyboard function that processes keyboard inputs : void(uint8 key)
    * @param process_mouse function that processes mouse inputs: <void(mouse_button left, mouse_button right, size_t mouse_x, size_t mouse_y)
    * @param process_mouse_motion function that processes mouse motion: <void(float x, float y) x,y in [-1,1] normalized cooridnates
    * @param x windows x position
    * @param y windows y position
    */
    void mainloop(std::string const& name, std::size_t w, std::size_t h, 
        std::function<void()> step, 
        std::function<void()> init = nullptr,
        std::function<void()> release = nullptr,
        std::function<void()> update_swap_chain = nullptr,
		std::function<void(std::uint8_t)> process_keyboard = nullptr,
        std::function<void(mouse_button, mouse_button, std::size_t, std::size_t)> process_mouse = nullptr,
        std::function<void(float, float)> process_mouse_motion = nullptr,
        std::size_t x = 0, std::size_t y = 0);

} // namespace dx12u



#endif // DX12U_APP_HPP
