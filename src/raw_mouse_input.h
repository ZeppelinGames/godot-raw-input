#ifndef RAW_MOUSE_INPUT_H
#define RAW_MOUSE_INPUT_H

#include <godot_cpp/classes/node.hpp>
#include <windows.h>

using namespace godot;

class RawMouseInput : public Node
{
    GDCLASS(RawMouseInput, Node);

public:
    static void _bind_methods();
    bool init();
    void _process(double delta);
    static void forward_raw_mouse(USHORT usFlags, ULONG ulButtons, USHORT usButtonFlags, USHORT usButtonData, ULONG ulRawButtons, LONG lLastX, LONG lLastY);

private:
    HWND hwnd = nullptr;
    WNDPROC original_wndproc = nullptr;
};

// Function declarations for GDExtension module initialization
extern "C" void initialize_rawmouseinput(ModuleInitializationLevel p_level);
extern "C" void uninitialize_rawmouseinput(ModuleInitializationLevel p_level);

#endif // RAW_MOUSE_INPUT_H
