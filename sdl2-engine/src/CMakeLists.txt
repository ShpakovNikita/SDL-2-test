cmake_minimum_required(VERSION 3.2)
project(SDLdynamic)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE game.cpp engine.cxx headers/engine.hxx)

set(PROJECT_LINK_LIB -lmingw32 -lSDL2main -lSDL2 -mwindows)
#-lmingw32  -mwindows

link_directories(${PROJECT_LIBS_DIR})

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIB})

message("Build!")