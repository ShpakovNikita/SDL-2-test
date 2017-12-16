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
constexpr int WINDOW_HEIGHT = 640;

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    std::unique_ptr<instance, void (*)(instance*)> player(
        create_player(std::vector<float>(), 0.0f, 0.0f, 0.0f), destroy_player);

    std::ifstream fin(VERTEX_FILE);
    assert(!!fin);

    triangle t_arr[2];
    fin >> t_arr[0] >> t_arr[1];

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    mode current_mode = mode::idle;

    float posX = 0.0f, posY = 0.0f;

    bool keys[17];
    for (int i = 0; i < 17; i++)
        keys[i] = false;

    std::vector<float> data;

    int speed = 1;
    float prev_frame = eng->GL_time();
    bool quit = false;
    while (!quit) {
        float delta_time = eng->GL_time() - prev_frame;
        prev_frame = eng->GL_time();
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

            if (eng->get_event_type() == event_type::pressed)
                keys[static_cast<int>(e)] = true;

            else
                keys[static_cast<int>(e) - 1] = false;
        }

        if (keys[static_cast<int>(event::left_pressed)])
            posX -= speed * delta_time;
        if (keys[static_cast<int>(event::right_pressed)])
            posX += speed * delta_time;
        if (keys[static_cast<int>(event::up_pressed)])
            posY -= speed * delta_time;
        if (keys[static_cast<int>(event::down_pressed)])
            posY += speed * delta_time;

        switch (current_mode) {
            case mode::idle: {
                eng->GL_clear_color();

                triangle add_t(vertex_2d(posX, posY, 0.0f, 0.0f),
                               vertex_2d(posX, posY, 0.0f, 0.0f),
                               vertex_2d(posX, posY, 0.0f, 0.0f));

                for (auto t : t_arr)
                    convert_triangle(t + add_t, data);

                eng->draw(data);

                data.clear();

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
                std::vector<float> d;
                convert_triangle(tr1, d);
                convert_triangle(tr2, d);

                eng->draw(d);

                eng->GL_swap_buffers();
            } break;
            case mode::look: {
                eng->GL_clear_color();

                std::ifstream fin(SIN_FILE);
                assert(!!fin);

                triangle tr1, tr2;
                fin >> tr1 >> tr2;

                float h = sin(eng->GL_time()) * 0.4f - 0.6f;
                float w = cos(eng->GL_time()) * 0.4f - 0.6f;

                for (auto& v : tr1.vertices) {
                    v.x_t += w;
                    v.y_t += h;
                }

                for (auto& v : tr2.vertices) {
                    v.x_t += w;
                    v.y_t += h;
                }

                std::vector<float> d;
                convert_triangle(tr1, d);
                convert_triangle(tr2, d);

                eng->draw(d);

                eng->GL_swap_buffers();
            } break;
            default:
                break;
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
