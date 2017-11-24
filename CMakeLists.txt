cmake_minimum_required(VERSION 3.2)
project(SDLdynamic)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE sdl2tests.cpp)
add_executable(${PROJECT_NAME} ${SOURCE})

#set(CMAKE_PREFIX_PATH C:\MinGWlibs\lib\x64) static?

target_link_libraries(${PROJECT_NAME} "-lSDL2main -lSDL2")
    

       