/*
 * engine.cxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#include "headers/engine.hxx"
#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include <algorithm>
#include <array>
#include <iostream>

static std::array<std::string, 17> event_names = {
    /// input events
    "left_pressed", "left_released", "right_pressed", "right_released",
    "up_pressed", "up_released", "down_pressed", "down_released",
    "select_pressed", "select_released", "start_pressed", "start_released",
    "button1_pressed", "button1_released", "button2_pressed",
    "button2_released",
    /// virtual console events
    "turn_off"};

const std::array<bind, 8> bindings{
    bind{SDLK_w, "up", event::up_pressed, event::up_released},
    bind{SDLK_a, "left", event::left_pressed, event::left_released},
    bind{SDLK_s, "down", event::down_pressed, event::down_released},
    bind{SDLK_d, "right", event::right_pressed, event::right_released},
    bind{SDLK_LCTRL, "button1", event::button1_pressed,
         event::button1_released},
    bind{SDLK_SPACE, "button2", event::button2_pressed,
         event::button2_released},
    bind{SDLK_ESCAPE, "select", event::select_pressed, event::select_released},
    bind{SDLK_RETURN, "start", event::start_pressed, event::start_released}};

std::ostream& operator<<(std::ostream& out, const SDL_version& v) {
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

std::ostream& operator<<(std::ostream& stream, const event e) {
    std::uint32_t value = static_cast<std::uint32_t>(e);
    std::uint32_t minimal = static_cast<std::uint32_t>(event::left_pressed);
    std::uint32_t maximal = static_cast<std::uint32_t>(event::turn_off);
    if (value >= minimal && value <= maximal) {    ///По идее вообще это не
                                                   ///должно никогда отрабоатать
                                                   ///так как сюда поступают
                                                   ///лишь уже отсеенные события
        stream << event_names[value];
        return stream;
    } else {
        throw std::runtime_error("too big event value");
    }
}

static bool check_input(const SDL_Event& e, const bind*& result) {
    using namespace std;

    const auto it =
        find_if(bindings.begin(), bindings.end(),
                [&](const bind& b) { return b.key == e.key.keysym.sym; });

    if (it != bindings.end()) {
        result = &(*it);
        return true;
    }
    return false;
}

namespace CHL {
engine::engine() {}
engine::~engine() {}

int engine::CHL_init(int width, int height) {
    SDL_version compiled = {0, 0, 0};
    SDL_version linked = {0, 0, 0};

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    if (SDL_COMPILEDVERSION !=
        SDL_VERSIONNUM(linked.major, linked.minor, linked.patch)) {
        std::cerr << "warning: SDL2 compiled and linked version mismatch: "
                  << compiled << " " << linked << std::endl;
    }

    const int init_result = SDL_Init(SDL_INIT_EVERYTHING);
    if (init_result != 0) {
        const char* err_message = SDL_GetError();
        std::cerr << "error: failed call SDL_Init: " << err_message
                  << std::endl;
        return EXIT_FAILURE;
    }

    this->window = SDL_CreateWindow("Chlorine-5", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, width, height,
                                    SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        const char* err_message = SDL_GetError();
        std::cerr << "error: failed call SDL_Init: " << err_message
                  << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    assert(gl_context != nullptr);

    int gl_major_ver = 0;
    int res = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_ver);
    assert(res == 0);

    int gl_minor_ver = 0;
    res = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_minor_ver);
    assert(res == 0);

    std::cerr << "Gl:" << gl_major_ver << '.' << gl_minor_ver << std::endl;

    glClearColor(0.f, 1.0, 0.f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
    return EXIT_SUCCESS;
}

void engine::CHL_exit() {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return (void)EXIT_SUCCESS;
}

bool engine::read_input(event& e) {
    SDL_Event event;

    if (SDL_PollEvent(&event)) {
        const bind* binding = nullptr;

        switch (event.type) {
            case SDL_QUIT:
                e = event::turn_off;
                return true;
            case SDL_KEYDOWN:
                if (check_input(event, binding)) {
                    e = binding->event_pressed;
                    return true;
                }
                break;
            case SDL_KEYUP:
                if (check_input(event, binding)) {
                    e = binding->event_released;
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return false;
}

}    // namespace CHL
