## Game engine
#### Made by Hidde Meiburg 

### Setup
All you need to run is the update_glslang_sources.py from the glslang library (```cd /lib/glslang``` and run it with ```python update_glslang_sources.py```)<br>
And then install the dependecies used by Vulkan and GLFW (see below)<br>
Then you can compile this gameengine with CMake<br>

!!!! This project is using c++26 features (reflection) !!!!<br>
These can be enabled as followed (in CMakeLists.txt):
```
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
(Right now only a Clang version by bloomberg (see [Github](https://github.com/bloomberg/clang-p2996/tree/purpose?tab=readme-ov-file)) supports the required features)

### Features
- Vulkan renderer
- TTF binary parser + TTF font program executor
- SDF maker for the text
- HTTP and Websocket server (with easy communication making use of c++26 reflection, see Network::WebsocketHandler)
- Impulse based physics
- Serialization (with c++26 reflection, also used for websocket communication)

### Libraries used
- asio          (async tcp sockets)
- entt          (entity component system)
- glfw          (multi platform window creation)
- glslang       (glsl shader compilation)
- stb-images    (image loading)
- FontAwesome   (nice looking html icons)

### Example dependency install command for Fedora
You need to install a c++ compiler (with c++26 support for reflection) + cmake (optionally doxygen for generating the docs). <br>
Then you also need to install the dependencies for:
- Vulkan see [vulkan-tutorial.com](https://vulkan-tutorial.com/Development_environment)
- GLFW see the depedencies section on [www.glfw.org](https://www.glfw.org/docs/3.3/compile.html)
```shell
sudo dnf upgrade -y &&\
sudo dnf install -y g++ &&\
sudo dnf install -y git &&\
sudo dnf install -y cmake &&\
sudo dnf install -y doxygen &&\
sudo dnf install -y python3 &&\
sudo dnf install -y gdb &&\
sudo dnf install -y libxkbcommon-devel && \
sudo dnf install -y libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel &&\
sudo dnf install -y vulkan-loader-devel &&\
sudo dnf install -y vulkan-validation-layers && \
sudo dnf install -y wayland-devel &&\
sudo dnf install -y libasan
```

### How to compile bloomberg clang for Fedora
The bloomberg's version of Clang (at the moment of writing) is the only compiler with c++26 reflection support<br>
It's binaries aren't available, so you need to compile it yourself.<br>
This is how to compile on a Fedora like system:
```shell

# Get the source
cd <to the directory you want to put the source>
git clone https://github.com/bloomberg/clang-p2996.git --branch p2996 --depth 1 ${name_of_source_directory}

# Compile clang, libc++ and the sanitizers
# An example of a target tripple is x86_64-redhat-linux
cd ${name_of_source_directory}
cmake -B ${Your_build_directory} -G "Unix Makefiles" -S llvm \
-DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra' \
-DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi;libunwind;compiler-rt' \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_DEFAULT_TARGET_TRIPLE=$(gcc -dumpmachine) \
-DCMAKE_INSTALL_PREFIX=/usr/local \
-DLIBCXX_HARDENING_MODE=extensive # This is need to generated the __config_site
cd ${Your_build_directory}
sudo make install -j12 # Compile with 12 cores

# Let the system be able to find libc++
echo "/usr/local/lib/$(gcc -dumpmachine)" | sudo tee /etc/ld.so.conf.d/libcxx-custom.conf
sudo ldconfig
```