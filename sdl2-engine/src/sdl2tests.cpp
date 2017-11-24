#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "headers/engine.hxx"

struct bind {
    bind(SDL_Keycode k, std::string n) : key(k), name(n) {}

    SDL_Keycode key;
    std::string name;
};

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine> game(new engine());
    engine eng = new engine();
    return EXIT_SUCCESS;
}
