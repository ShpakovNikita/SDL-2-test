#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <math.h>

#include "headers/engine.hxx"

enum class mode { draw, look, idle };

const std::string VERTEX_FILE = "vertices.txt";
const std::string SIN_FILE = "sin.txt";

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 1024;

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    mode current_mode = mode::idle;

    bool quit = false;
    while (!quit) {
        event e;

        while (eng->read_input(e)) {
            std::cout << e << std::endl;
            switch (e) {
                case event::turn_off:
                    quit = true;
                    break;
                case event::select_pressed:
                    current_mode = mode::draw;
                    break;
                case event::start_pressed:
                    current_mode = mode::look;
                    break;
                case event::button1_pressed:
                    current_mode = mode::idle;
                    break;
                default:
                    break;
            }
        }

        switch (current_mode) {
            case mode::idle: {
                eng->GL_clear_color();

                std::ifstream fin(VERTEX_FILE);
                assert(!!fin);

                triangle tr1, tr2;
                fin >> tr1 >> tr2;

                eng->draw_triangle(tr1);
                eng->draw_triangle(tr2);

                eng->GL_swap_buffers();
            } break;
            case mode::draw: {
                eng->GL_clear_color();

                std::ifstream fin(VERTEX_FILE);
                assert(!!fin);

                float alpha = sin(eng->GL_time()) / 2. + 0.5f;
                triangle t1q, t2q, t1r, t2r;
                fin >> t1q >> t2q >> t1r >> t2r;

                // draw events
                triangle tr1 = blend(t1q, t1r, alpha);
                triangle tr2 = blend(t2q, t2r, alpha);
                eng->draw_triangle(tr1);
                eng->draw_triangle(tr2);

                eng->GL_swap_buffers();
            } break;
            case mode::look: {
                eng->GL_clear_color();

                std::ifstream fin(SIN_FILE);
                assert(!!fin);

                triangle tr1, tr2;
                fin >> tr1 >> tr2;

                float h = sin(eng->GL_time()) * 0.3f;
                float w = cos(eng->GL_time()) * 0.3f;

                for (auto& v : tr1.vertices) {
                    v.x_t += w;
                    v.y_t += h;
                }

                for (auto& v : tr2.vertices) {
                    v.x_t += w;
                    v.y_t += h;
                }

                eng->draw_triangle(tr1);
                eng->draw_triangle(tr2);

                eng->GL_swap_buffers();
            } break;
            default:
                break;
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
