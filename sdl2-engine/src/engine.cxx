/*
 * engine.cxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#include "headers/engine.hxx"
#include <iostream>

std::ostream& operator<<(std::ostream& out, const SDL_version& v) {
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

namespace CHL {
engine::engine() {}
engine::~engine() {}

int engine::CHL_Init(std::string /*configure*/) {
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

    window =
        SDL_CreateWindow("Chlorine-5", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        const char* err_message = SDL_GetError();
        std::cerr << "error: failed call SDL_Init: " << err_message
                  << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void engine::CHL_Exit() {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return (void)EXIT_SUCCESS;
}

void engine::read_input() {
    return (void)EXIT_SUCCESS;
}

}    // namespace CHL
