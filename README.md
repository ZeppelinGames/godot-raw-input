# Godot Raw Input
WndProc raw input handling for Godot 4.4.  
**Only works on Windows build targets (for obvious reasons)**

### Compilation
Follow basic GDExtension compilation steps as per [docs](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)  

1. Clone `godot-cpp` to root dir from [godotengine/godot-cpp](https://github.com/godotengine/godot-cpp)  
2. Run `scons`  
3. Will build `rawmouseinput`
4. Copy files in `./rawmouseinput/bin` to `./<project>/bin/`
5. Create or copy preexisting `.gdextension` file into `./<project>/bin/`

### Features
- Basic raw mouse input handling