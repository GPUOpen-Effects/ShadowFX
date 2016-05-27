
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
* @file      app.cpp
* @brief     dx12 utilities app implementation
*/

#include <app.hpp>
#include <array>
#include <iostream>
#include <dx12u.hpp>
#include <hidusage.h>

namespace
{
    namespace global
    {
        HWND hwnd = 0;
        std::size_t width = 0;
        std::size_t height = 0;
        std::array<bool, 255> vk_pressed = { false };
        std::function<void()> update_swap_chain{};
		std::function<void(std::uint8_t)> process_keyboard{};
        std::function<void(dx12u::mouse_button, dx12u::mouse_button, std::size_t, std::size_t)> process_mouse{};
        std::function<void(float, float)> process_mouse_motion{};
        std::function<void()> release{};
        bool quit_application = false;
    }

    void cleanup()
    {
        if (global::release)
        {
            global::release();
        }
        global::update_swap_chain = nullptr;
        global::process_keyboard = nullptr;
        global::process_mouse = nullptr;
        global::process_mouse_motion = nullptr;
        global::release = nullptr;
    }
}

LRESULT CALLBACK process_win32_msg(HWND hwnd, uint32_t msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_CLOSE)
    {
        cleanup();
        global::quit_application = true;
        return 0;
    }

    if (msg == WM_DESTROY)
    {
        // no cleanup here as HWND is not valid anymore
        global::quit_application = true;
        return 0;
    }

    if (msg == WM_KEYDOWN)
    {
        global::vk_pressed[wparam & 255] = true;

		if (global::process_keyboard)
		{
			global::process_keyboard(static_cast<std::uint8_t>(wparam));
		}

        if (wparam == VK_ESCAPE)
        {
            cleanup();
            global::quit_application = true;
        }
        return 0;
    }

    if (msg == WM_KEYUP)
    {
        global::vk_pressed[wparam & 255] = false;
        return 0;
    }

    if (msg == WM_SIZE)
    {
        global::width = LOWORD(lparam);
        global::height = HIWORD(lparam);
        if (global::update_swap_chain != nullptr)
        {
            global::update_swap_chain();
        }
        return 0;
    }

    if (msg == WM_INPUT && global::process_mouse_motion)
    {
        RAWINPUT raw_input{};
        UINT sz = sizeof(raw_input);
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &raw_input, &sz, sizeof(RAWINPUTHEADER));
        if (raw_input.header.dwType == RIM_TYPEMOUSE)
        {
            float x = raw_input.data.mouse.lLastX / (static_cast<float>(global::width) * .5f);
            float y = raw_input.data.mouse.lLastY / (static_cast<float>(global::height) * .5f);
            global::process_mouse_motion(x, -y);
        }
        return 0;
    }

    if (global::process_mouse)
    {
        if (msg == WM_LBUTTONDOWN)
        {
            global::process_mouse(dx12u::mouse_button::down, dx12u::mouse_button::none, (static_cast<size_t>(lparam) & 0xffff), ((static_cast<size_t>(lparam)  >> 16) & 0xffff));
            return 0;
        }
        if (msg == WM_LBUTTONUP)
        {
            global::process_mouse(dx12u::mouse_button::up, dx12u::mouse_button::none, (static_cast<size_t>(lparam) & 0xffff), ((static_cast<size_t>(lparam) >> 16) & 0xffff));
            return 0;
        }
        if (msg == WM_RBUTTONDOWN)
        {
            global::process_mouse(dx12u::mouse_button::none, dx12u::mouse_button::down, (static_cast<size_t>(lparam) & 0xffff), ((static_cast<size_t>(lparam) >> 16) & 0xffff));
            return 0;
        }
        if (msg == WM_RBUTTONUP)
        {
            global::process_mouse(dx12u::mouse_button::none, dx12u::mouse_button::up, (static_cast<size_t>(lparam) & 0xffff), ((static_cast<size_t>(lparam) >> 16) & 0xffff));
            return 0;
        }
        if (msg == WM_MOUSEMOVE)
        {
            global::process_mouse(dx12u::mouse_button::none, dx12u::mouse_button::none, (static_cast<size_t>(lparam) & 0xffff), ((static_cast<size_t>(lparam) >> 16) & 0xffff));
            return 0;
        }
    }

    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

namespace dx12u
{
    std::size_t get_window_width() noexcept
    {
        return global::width;
    }

    std::size_t get_window_height() noexcept
    {
        return global::height;
    }

    bool is_key_pressed(std::uint8_t vk) noexcept
    {
        return global::vk_pressed[vk];
    }

    HWND get_hwnd() noexcept
    {
        return global::hwnd;
    }

    void quit_app()
    {
        global::quit_application = true;
    }

    void resize_window(std::size_t w, std::size_t h)
    {
        if (w == global::width && h == global::height)
        {
            return;
        }
        RECT rc = { static_cast<LONG>(0), static_cast<LONG>(0), static_cast<LONG>(w), static_cast<LONG>(h) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        if (!MoveWindow(global::hwnd, 0, 0, rc.right - rc.left, rc.bottom - rc.top, false))
        {
            throw error{ "resize window failed" };
        }
    }

    void set_mouse_capture(bool capture)
    {
        if (capture)
        {
            SetCapture(global::hwnd);
            ShowCursor(false);
        }
        else
        {
            ReleaseCapture();
            ShowCursor(true);
        }
    }

    void mainloop(std::string const& name, std::size_t w, std::size_t h, 
		std::function<void()> step, 
		std::function<void()> init, 
		std::function<void()> release, 
		std::function<void()> update_swap_chain, 
		std::function<void(std::uint8_t)> process_keyboard,
        std::function<void(mouse_button, mouse_button, std::size_t, std::size_t)> process_mouse,
        std::function<void(float, float)> process_mouse_motion,
		std::size_t x, std::size_t y)
    {
        if (global::hwnd != 0)
        {
            throw error{ "only a single window can be created" };
        }

        if (!step)
        {
            throw error{ "a step function must exist" };
        }

        HINSTANCE hinstance = GetModuleHandle(NULL);
        WNDCLASSEXA wcex;
        memset(&wcex, 0, sizeof(WNDCLASSEXA));
        wcex.cbSize = sizeof(WNDCLASSEXA);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = process_win32_msg;
        wcex.hInstance = hinstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        // wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = name.c_str();
        if (!RegisterClassExA(&wcex))
        {
            throw std::runtime_error{ "window creation failure" };
        }
        RECT rc = { static_cast<LONG>(x), static_cast<LONG>(y), static_cast<LONG>(w), static_cast<LONG>(h) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        auto hwnd = CreateWindowA(wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hinstance, NULL);
        if (!hwnd)
        {
            throw std::runtime_error{ "window creation failure" };
        }
        ShowWindow(hwnd, SW_SHOW);
        global::hwnd = hwnd;
        global::width = w;
        global::height = h;

        global::update_swap_chain = update_swap_chain;
		global::process_keyboard = process_keyboard;
        global::process_mouse = process_mouse;
        global::process_mouse_motion = process_mouse_motion;
        global::release = release;

        // init input
        RAWINPUTDEVICE rid[2];
        rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
        rid[0].dwFlags = 0;
        rid[0].hwndTarget = hwnd;
        rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
        rid[1].dwFlags = 0;
        rid[1].hwndTarget = hwnd;
        RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));

        // init device
        if (init)
        {
            init();
        }

        MSG msg = { 0 };
        while (true)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (global::quit_application)
            {
                return;
            }

            // update and render
            step();
        }
    }
} // namespace
