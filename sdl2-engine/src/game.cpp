#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "headers/engine.hxx"

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;

    engine* const eng = new engine();
    eng->CHL_init("");

    bool state = true;
    while (state) {
        event e;

        while (eng->read_input(e)) {
            std::cout << e << std::endl;
            switch (e) {
                case event::turn_off:
                    state = false;
                    break;
                default:
                    break;
            }
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
