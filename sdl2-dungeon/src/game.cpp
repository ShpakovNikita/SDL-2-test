#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <math.h>

#include "headers/engine.hxx"
#include "dungeon.cpp"

enum class mode { draw, look, idle };

const std::string VERTEX_FILE = "vertices.txt";
const std::string SIN_FILE = "sin.txt";

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 640;

constexpr int TILE_SIZE = 320;

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

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE);
    mode current_mode = mode::idle;

    float posX = 0.0f, posY = 0.0f;

    bool keys[17];
    for (int i = 0; i < 17; i++)
        keys[i] = false;

    std::vector<float> data;
    for (auto t : t_arr)
        convert_triangle(t, data);

    std::vector<instance*> bricks;

    DungeonGenerator generator;
    auto map = generator.Generate();
    std::vector<int> tile_set = map.Print();

    //    for (int y = 0; y < tile_set[tile_set.size() - 1]; y++) {
    //        for (int x = 0; x < tile_set[tile_set.size() - 2]; x++) {
    //            std::cout << *(tile_set.begin() +
    //                           y * tile_set[tile_set.size() - 1] + x)
    //                      << " ";
    //            if (*(tile_set.begin() + y * tile_set[tile_set.size() - 1] +
    //            x) ==
    //                1)
    //                bricks.insert(
    //                    bricks.end(),
    //                    create_wall(
    //                        data, -1.0f + (x * TILE_SIZE) /
    //                        (float)WINDOW_WIDTH, 1.0f - (y * TILE_SIZE) /
    //                        (float)WINDOW_HEIGHT, 0.0f));
    //        }
    //        std::cout << std::endl;
    //    }

    bricks.insert(bricks.end(), create_wall(data, 1, -2, 0.0f));
    bricks.insert(bricks.end(), create_wall(data, 1, -1, 0.0f));
    bricks.insert(bricks.end(), create_wall(data, 1, 0, 0.0f));
    bricks.insert(bricks.end(), create_wall(data, 1, 1, 0.0f));
    bricks.insert(bricks.end(), create_wall(data, 1, 2, 0.0f));

    std::cout << tile_set[tile_set.size() - 1] << " "
              << tile_set[tile_set.size() - 2] << std::endl;

    data.clear();

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

        switch (current_mode) {
            case mode::idle: {
                if (keys[static_cast<int>(event::left_pressed)])
                    posX -= speed * delta_time;
                if (keys[static_cast<int>(event::right_pressed)])
                    posX += speed * delta_time;
                if (keys[static_cast<int>(event::up_pressed)])
                    posY -= speed * delta_time;
                if (keys[static_cast<int>(event::down_pressed)])
                    posY += speed * delta_time;

                eng->GL_clear_color();

                triangle add_t(vertex_2d(posX, posY, 0.0f, 0.0f),
                               vertex_2d(posX, posY, 0.0f, 0.0f),
                               vertex_2d(posX, posY, 0.0f, 0.0f));

                for (auto t : t_arr)
                    convert_triangle(t + add_t, data);

                for (auto brick : bricks)
                    eng->add_object(brick->get_vector());

                eng->add_object(data);
                eng->draw();

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

                eng->add_object(d);
                eng->draw();

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

                eng->add_object(d);
                eng->draw();

                eng->GL_swap_buffers();
            } break;
            default:
                break;
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
