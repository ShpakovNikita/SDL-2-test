cmake_minimum_required(VERSION 3.2)
project(SDLdynamic)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE game.cpp engine.cxx headers/engine.hxx headers/picopng.hxx headers/audio.h dungeon.cpp)

if(UNIX)
    set(PROJECT_LINK_LIB -lSDL2main -lSDL2)
else()
    set(PROJECT_LINK_LIB -lmingw32 -lglew32 -lSDL2main -lSDL2 -mwindows -lopengl32)
endif(UNIX)


link_directories(${PROJECT_LIBS_DIR})

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIB})

message("Build!")
