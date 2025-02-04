// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: MIT
#include "Window.h"
#include <format>
#include <Core/source/infra/Logging.h>
#include <Core/source/infra/util/Util.h>
#include "MessageMap.h"
#include "WndClass.h"

namespace p2c::win
{
    Window::Window(std::wstring title, DWORD styles)
        :
        title{ std::move(title) }, 
        styles{ styles }
    {}

    Window::~Window()
    {
        pmlog_verb(v::window)(std::format(L"window dying hwn:[{:8x}] tit:[{}]", (uint64_t)hWnd, GetTitle()));
        if (DestroyWindow(hWnd) == FALSE)
        {
            pmlog_error().hr();
        }
    }

    HWND Window::GetHandle()
    {
        return hWnd;
    }

    const std::wstring &Window::GetTitle() const
    {
        return title;
    }

    gfx::Vec2I Window::GetPosition() const
    {
        RECT rect{}; 
        if (GetWindowRect(hWnd, &rect) == FALSE)
        {
            pmlog_warn(std::format(L"failed to get window rect {}", GetTitle())).hr();
        }
        return { rect.left, rect.top };
    }

    void Window::Move(gfx::Vec2I pos)
    {
        pmlog_verb(v::window)(std::format(L"pos:[{},{}]", pos.x, pos.y));
        if (SetWindowPos(hWnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE) == FALSE)
        {
            pmlog_warn(std::format(L"failed to move window {}", GetTitle())).hr();
        }
    }

    void Window::Reorder(HWND base)
    {
        pmlog_verb(v::window)(std::format(L"hwnd:{:8x}", (uint64_t)base));
        if (SetWindowPos(hWnd, GetNextWindow(base, GW_HWNDPREV), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE)
        {
            pmlog_warn(std::format(L"failed to reorder window {}", GetTitle())).hr();
        }
    }

    void Window::ReorderBehind(HWND base)
    {
        pmlog_verb(v::window)(std::format(L"hwnd:{:8x}", (uint64_t)base));
        if (SetWindowPos(hWnd, base, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE) {
            pmlog_warn(std::format(L"failed to reorder window {} behind", GetTitle())).hr();
        }
    }

    void Window::SetTopmost()
    {
        pmlog_verb(v::window)();
        if (SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE)
        {
            pmlog_warn(std::format(L"failed to make window topmost {}", GetTitle())).hr();
        }
    }

    void Window::ClearTopmost()
    {
        pmlog_verb(v::window)();
        if (SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE) == FALSE)
        {
            pmlog_warn(std::format(L"failed to make window non-topmost {}", GetTitle())).hr();
        }
    }

    void Window::Close()
    {
        pmlog_verb(v::window)();
        if (PostMessage(hWnd, WM_CLOSE, 0, 0) == FALSE)
        {
            pmlog_warn(std::format(L"failed to close window {}", GetTitle())).hr();
        }
    }

    LRESULT Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        LogMessage(msg, wParam, lParam);

        switch (msg)
        {
        case WM_SETTEXT:
            title = reinterpret_cast<wchar_t*>(lParam);
            break;
        }

        if (const auto result = CustomHandleMessage(msg, wParam, lParam))
        {
            return *result;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void Window::SetMessageLogging(bool enable)
    {
        loggingMessages = enable;
    }

    void Window::Hide()
    {
        pmlog_verb(v::window)();
        ShowWindow(hWnd, SW_HIDE);
    }

    void Window::Show()
    {
        pmlog_verb(v::window)();
        ShowWindow(hWnd, SW_SHOWNOACTIVATE);
    }

    void Window::Resize(gfx::DimensionsI clientSize)
    {
        pmlog_verb(v::window)();
        const auto size = ComputeWindowDimensions(clientSize);
        if (SetWindowPos(hWnd, nullptr, 0, 0, size.width, size.height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE) == FALSE)
        {
            pmlog_warn(std::format(L"failed to resize window {}", GetTitle())).hr();
        }
    }

    gfx::DimensionsI Window::ComputeWindowDimensions(gfx::DimensionsI clientArea)
    {
        RECT wr{
            .left = 0,
            .top = 0,
            .right = clientArea.width,
            .bottom = clientArea.height,
        };
        if (AdjustWindowRect(&wr, styles, FALSE) == FALSE)
        {
            pmlog_warn(L"Failed to adjust window rect").hr();
        }
        return RectToDims(wr);
    }

    ATOM Window::GetDefaultClass()
    {
        static WndClass defaultClass;
        return defaultClass.GetAtom();
    }

    std::optional<LRESULT> Window::CustomHandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void Window::SetHandle(HWND hWnd_)
    {
        pmlog_verb(v::window)(std::format(L"hwnd:{:8x}", (uint64_t)hWnd_));
        if (hWnd != nullptr)
        {
            pmlog_warn(L"handle already set for window");
        }
        hWnd = hWnd_;
    }

    void Window::LogMessage(DWORD msg, LPARAM, WPARAM)
    {
        if (loggingMessages)
        {
            pmlog_info(std::format(L"WinMsg@[{}] : {}", GetTitle(), LookupMessageName(msg)));
        }
    }

    const wchar_t *Window::LookupMessageName(DWORD msg)
    {
        static MessageMap messageMap;
        return messageMap.GetMessageName(msg);
    }

    gfx::DimensionsI RectToDims(const RECT& r)
    {
        return { r.right - r.left, r.bottom - r.top };
    }

    gfx::RectI RectToRectI(const RECT& r)
    {
        return { r.left, r.top, r.right, r.bottom };
    }
}
