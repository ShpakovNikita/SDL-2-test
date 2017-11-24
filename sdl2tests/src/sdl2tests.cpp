#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>

#include <SDL2/SDL.h>

std::ostream& operator<<(std::ostream& out, const SDL_version& v) {
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);

    return out;
}

struct bind {
    bind(SDL_Keycode k, std::string n) : key(k), name(n) {}

    SDL_Keycode key;
    std::string name;
};

void process_input(SDL_Event e) {
    std::array<bind, 8> bindings{bind(SDLK_w, "up"),
                                 bind(SDLK_a, "left"),
                                 bind(SDLK_s, "down"),
                                 bind(SDLK_d, "right"),
                                 bind(SDLK_ESCAPE, "esc (select)"),
                                 bind(SDLK_RETURN, "enter (start)"),
                                 bind(SDLK_j, "a button"),
                                 bind(SDLK_k, "b button")};

    auto it =
        std::find_if(bindings.begin(), bindings.end(),
                     [&](const bind& b) { return b.key == e.key.keysym.sym; });

    if (it != bindings.end()) {
        std::cout << it->name;
        if (e.type == SDL_KEYDOWN)
            std::cout << " is pressed" << std::endl;

        else
            std::cout << " is released" << std::endl;
    }

    return (void)EXIT_SUCCESS;
}

int main(int /*argc*/, char* /*argv*/ []) {
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

    SDL_Window* const window =
        SDL_CreateWindow("Chlorine-5", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        const char* err_message = SDL_GetError();
        std::cerr << "error: failed call SDL_Init: " << err_message
                  << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_bool state = SDL_TRUE;
    while (state) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYUP:

                case SDL_KEYDOWN:
                    process_input(event);
                    break;
                case SDL_QUIT:
                    state = SDL_FALSE;
                    break;
                default:
                    break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
