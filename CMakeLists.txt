cmake_minimum_required(VERSION 3.10)
project(BrickGame)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义ENET_STATIC宏，告诉编译器使用静态库
add_definitions(-DENET_STATIC)

# 精准匹配你的raylib路径和版本（Windows下保持原来的路径即可）
set(RAYLIB_DIR "E:/software/raylib")
include_directories(${RAYLIB_DIR}/include)
link_directories(${RAYLIB_DIR}/lib)

# ENet网络库配置
set(ENET_DIR "E:/software/enet")
include_directories(${ENET_DIR}/include)
link_directories(${ENET_DIR}/lib)

# 你的游戏源文件（源文件都在src目录，所以保留src/前缀）
set(SOURCES
    src/main.cpp
    src/Ball.cpp
    src/Paddle.cpp
    src/Brick.cpp
    src/Game.cpp
    src/PowerUp.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库（顺序很重要！ENet必须在它的依赖库之前）
target_link_libraries(${PROJECT_NAME}
    raylib
    enet
    winmm
    gdi32
    opengl32
    ws2_32
    iphlpapi
)

# 单元测试目标
add_executable(test
    src/test.cpp
    src/Ball.cpp
    src/Paddle.cpp
    src/Brick.cpp
)
target_include_directories(test PRIVATE ${RAYLIB_DIR}/include)
target_link_libraries(test PRIVATE raylib winmm gdi32 opengl32)