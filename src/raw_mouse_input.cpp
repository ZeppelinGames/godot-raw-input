#include "raw_mouse_input.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <vector>
#include <string>
#include <cstdio>
#include <mutex>
#include <queue>

using namespace godot;

struct RawMouseEvent
{
    godot::String guid;
    USHORT usFlags;
    ULONG ulButtons;
    USHORT usButtonFlags;
    USHORT usButtonData;
    ULONG ulRawButtons;
    LONG lLastX;
    LONG lLastY;
};

static HWND hwnd = nullptr;
static WNDPROC original_wndproc = nullptr;
static RawMouseInput *raw_mouse_input_instance = nullptr;
std::mutex raw_mouse_mutex;
std::queue<RawMouseEvent> raw_mouse_queue;

LRESULT CALLBACK CustomWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void RawMouseInput::_bind_methods()
{
    OutputDebugStringA("Binding RawMouseInput");
    ADD_SIGNAL(MethodInfo("raw_mouse",
                          PropertyInfo(godot::Variant::STRING, "guid"),
                          PropertyInfo(godot::Variant::INT, "usFlags"),
                          PropertyInfo(godot::Variant::INT, "ulButtons"),
                          PropertyInfo(godot::Variant::INT, "usButtonFlags"),
                          PropertyInfo(godot::Variant::INT, "usButtonData"),
                          PropertyInfo(godot::Variant::INT, "ulRawButtons"),
                          PropertyInfo(godot::Variant::INT, "lLastX"),
                          PropertyInfo(godot::Variant::INT, "lLastY")));
}

bool RawMouseInput::init()
{
    OutputDebugStringA("Initialising RawMouseInput");
    raw_mouse_input_instance = this;

    DisplayServer *ds = DisplayServer::get_singleton();
    hwnd = reinterpret_cast<HWND>(ds->window_get_native_handle(godot::DisplayServer::HandleType::WINDOW_HANDLE));

    if (!hwnd)
    {
        OutputDebugStringA("Failed to get native window handle");
        return false;
    }
    else
    {
        OutputDebugStringA("Got HWND");
    }

    if (hwnd && !original_wndproc)
    {
        original_wndproc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CustomWndProc)));
        if (!original_wndproc)
        {
            OutputDebugStringA("Failed to hook WndProc");
            return false;
        }
        else
        {
            OutputDebugStringA("Successfully hooked WndProc");
        }

        RAWINPUTDEVICE rid;
        rid.usUsagePage = 0x01; // Generic desktop controls
        rid.usUsage = 0x02;     // Mouse
        rid.dwFlags = RIDEV_INPUTSINK;
        rid.hwndTarget = hwnd;
        bool registered = RegisterRawInputDevices(&rid, 1, sizeof(rid));
        if (!registered)
        {
            OutputDebugStringA("Failed to register raw input device");
            return false;
        }
        else
        {
            OutputDebugStringA("Registered raw input device");
        }
    }

    set_process(true);
    return true;
}

void RawMouseInput::_process(double delta)
{
    // Example: print or send to Godot
    std::lock_guard<std::mutex> lock(raw_mouse_mutex);
    while (!raw_mouse_queue.empty())
    {
        RawMouseEvent evt = raw_mouse_queue.front();
        raw_mouse_queue.pop();
        emit_signal("raw_mouse",
                    evt.guid,
                    (int)evt.usFlags,
                    (int)evt.ulButtons,
                    (int)evt.usButtonFlags,
                    (int)evt.usButtonData,
                    (int)evt.ulRawButtons,
                    (int)evt.lLastX,
                    (int)evt.lLastY);
    }
}

String GetDeviceName(HANDLE &hDevice) {
    UINT nameSize = 0;
    GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, nullptr, &nameSize);
    if (nameSize == 0) return "Unknown";

    std::vector<wchar_t> deviceName(nameSize);
    if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, deviceName.data(), &nameSize) <= 0) {   
        return "Invalid";
    }

    // Convert wide string to UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, deviceName.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8DeviceName(utf8Size, 0);
    WideCharToMultiByte(CP_UTF8, 0, deviceName.data(), -1, utf8DeviceName.data(), utf8Size, nullptr, nullptr);

    return String(utf8DeviceName.c_str());
}

LRESULT CALLBACK CustomWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INPUT)
    {
        OutputDebugStringA("WM_INPUT received\n");

        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
        std::vector<BYTE> lpb(dwSize);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
        {
            RAWINPUT *raw = (RAWINPUT *)lpb.data();
            HANDLE hDevice = raw->header.hDevice;

            String deviceName = GetDeviceName(hDevice);

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                RawMouseEvent evt = {
                    deviceName,
                    raw->data.mouse.usFlags,
                    raw->data.mouse.ulButtons,
                    raw->data.mouse.usButtonFlags,
                    raw->data.mouse.usButtonData,
                    raw->data.mouse.ulRawButtons,
                    raw->data.mouse.lLastX,
                    raw->data.mouse.lLastY};

                std::lock_guard<std::mutex> lock(raw_mouse_mutex);
                raw_mouse_queue.push(evt);
            }
        }
    }
    return CallWindowProc(original_wndproc, hWnd, msg, wParam, lParam);
}

void RawMouseInput::_enter_tree() {
    if (raw_mouse_input_instance) {
        OutputDebugStringA("RawMouseInput already initialized.\n");
        return;
    }

    init();
}

void RawMouseInput::_exit_tree() {
    if (raw_mouse_input_instance == this) {
        raw_mouse_input_instance = nullptr;
        set_process(false);

        if (original_wndproc && hwnd) {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)original_wndproc);
            original_wndproc = nullptr;
            hwnd = nullptr;
        }
    }
}

void initialize_rawmouseinput(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        ClassDB::register_class<RawMouseInput>();
    }
}

void uninitialize_rawmouseinput(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
    }
}