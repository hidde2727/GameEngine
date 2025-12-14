## Game engine
#### Made by Hidde Meiburg 

### Setup
All you need to run is the update_glslang_sources.py from the glslang library (/lib/glslang and run it)
(And you need to install the dependencies for the libraries that were used, see below)
Then you can compile this gameengine with CMake

!!!! This project is using c++26 features !!!!
These can be enabled as followed (in CMakeLists.txt):
```
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
(Right now only Clang supports the required features)

### Features
- Vulkan renderer
- TTF binary parser + TTF font program executor
- SDF maker for the text
- HTTP and Websocket server
- Impulse based physics
- Serialization

### Libraries used
- asio          (async tcp sockets)
- entt          (entity component system)
- glfw          (multi platform window creation)
- glslang       (glsl shader compilation)
- stb-images    (image loading)

### Example dependency install command for Fedora
```shell
sudo dnf install gcc-c++ &&\
sudo dnf install git &&\
sudo dnf install cmake &&\
sudo dnf install libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel &&\
sudo dnf install vulkan-tools &&\
sudo dnf install vulkan-loader-devel &&\
sudo dnf install mesa-vulkan-drivers vulkan-validation-layers-devel &&\
sudo dnf install wayland-devel && \
sudo dnf install libxkbcommon-devel && \
sudo dnf install vulkan-validation-layers && \
sudo dnf install gdb
```
