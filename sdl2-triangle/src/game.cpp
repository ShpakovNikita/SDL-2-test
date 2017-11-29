#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "headers/engine.hxx"

constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT);

    bool quit = false;
    while (!quit) {
        event e;

        while (eng->read_input(e)) {
            std::cout << e << std::endl;
            switch (e) {
                case event::turn_off:
                    quit = true;
                    break;
                default:
                    break;
            }
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
