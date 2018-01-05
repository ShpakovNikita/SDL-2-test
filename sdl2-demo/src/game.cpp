#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <math.h>
#include <cstdlib>
#include <time.h>

#include "headers/engine.hxx"
#include "dungeon.cpp"

enum class mode { draw, look, idle };

const std::string SND_FOLDER = "sounds\\";

const std::string START_MUSIC = "main.wav";
const std::string IDLE_SOUND = "idle.wav";
const std::string MOVE_SOUND = "move.wav";

const std::string VERTEX_FILE = "vertices.txt";
const std::string SIN_FILE = "sin.txt";

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 640;

constexpr int TILE_SIZE = 32;

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    texture* brick_tex = new texture();
    brick_tex->load_texture("brick.png");

    std::ifstream fin(VERTEX_FILE);
    assert(!!fin);

    triangle t_arr[2];
    fin >> t_arr[0] >> t_arr[1];

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE);
    mode current_mode = mode::idle;

    bool keys[17];
    for (int i = 0; i < 17; i++)
        keys[i] = false;

    std::vector<float> data;
    for (auto t : t_arr)
        convert_triangle(t, data);

    std::vector<instance*> bricks;

    int x_size = 34, y_size = 20;
    DungeonGenerator generator(x_size, y_size);
    auto map = generator.Generate();
    std::vector<int> tile_set = map.Print();

    int X_MAP = WINDOW_WIDTH / TILE_SIZE;
    int Y_MAP = WINDOW_HEIGHT / TILE_SIZE;

    bool placed = false;
    std::unique_ptr<life_form, void (*)(life_form*)> player(
        create_player(data, 0.0f, 0.0f, 0.0f), destroy_player);
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            std::cout << *(tile_set.begin() + y * x_size + x) << " ";
            if (*(tile_set.begin() + y * x_size + x) != 0)
                bricks.insert(bricks.end(),
                              create_wall(data, -X_MAP + 2 * x + 1,
                                          -Y_MAP + 2 * y + 1, 0.0f));
            else if (!placed && *(tile_set.begin() + y * x_size + x) == 0) {
                std::cout << *(tile_set.begin() + y * x_size + x) << std::endl;
                player->position.x = -X_MAP + 2 * x + 1;
                player->position.y = -Y_MAP + 2 * y + 1;
                placed = true;
            }
        }
        std::cout << std::endl;
    }

    sound start_music(SND_FOLDER + START_MUSIC);
    sound idle_sound(SND_FOLDER + IDLE_SOUND);
    sound move_sound(SND_FOLDER + MOVE_SOUND);
    idle_sound.play_always();
    //    start_music.play_always();

    //    bricks.insert(bricks.end(), create_wall(data, 1, -2, 0.0f));
    //    bricks.insert(bricks.end(), create_wall(data, 1, -1, 0.0f));
    //    bricks.insert(bricks.end(), create_wall(data, 1, 0, 0.0f));
    //    bricks.insert(bricks.end(), create_wall(data, 1, 1, 0.0f));
    //    bricks.insert(bricks.end(), create_wall(data, 1, 2, 0.0f));
    //    bricks.insert(bricks.end(), create_wall(data, 0 - X_MAP, 0 + Y_MAP,
    //    0.0f)); bricks.insert(bricks.end(), create_wall(data, 0 - X_MAP, 0 +
    //    Y_MAP, 0.0f));

    bool one_time_change = true;

    //    std::cout << tile_set[tile_set.size() - 1] << " "
    //              << tile_set[tile_set.size() - 2] << std::endl;

    data.clear();

    int speed = 4;
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

            if (eng->get_event_type() == event_type::pressed) {
                keys[static_cast<int>(e)] = true;
            }

            else {
                keys[static_cast<int>(e) - 1] = false;
            }
        }

        switch (current_mode) {
            case mode::idle: {
                bool moved = false;
                if (keys[static_cast<int>(event::left_pressed)]) {
                    moved = true;
                    player->position.x -= speed * delta_time;
                }
                if (keys[static_cast<int>(event::right_pressed)]) {
                    moved = true;
                    player->position.x += speed * delta_time;
                }
                if (keys[static_cast<int>(event::up_pressed)]) {
                    moved = true;
                    player->position.y -= speed * delta_time;
                }
                if (keys[static_cast<int>(event::down_pressed)]) {
                    moved = true;
                    player->position.y += speed * delta_time;
                }

                if (moved && one_time_change) {
                    idle_sound.stop();
                    move_sound.play_always();
                    std::cout << "move" << std::endl;
                    one_time_change = false;
                }

                if (!moved && !one_time_change) {
                    idle_sound.play_always();
                    move_sound.stop();
                    std::cout << "stop" << std::endl;
                    one_time_change = true;
                }

                eng->GL_clear_color();

                //                triangle add_t(vertex_2d(player->position.x,
                //                player->position.y,
                //                                         0.0f, 0.0f),
                //                               vertex_2d(player->position.x,
                //                               player->position.y,
                //                                         0.0f, 0.0f),
                //                               vertex_2d(player->position.x,
                //                               player->position.y,
                //                                         0.0f, 0.0f));
                //
                //                for (auto t : t_arr)
                //                    convert_triangle(t + add_t, data);

                for (auto brick : bricks)
                    eng->add_object(brick);

                eng->draw(brick_tex);

                eng->add_object(player.get());
                eng->draw(brick_tex);

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

                //                eng->add_object(d);
                eng->draw(brick_tex);

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

                //                eng->add_object(d);
                eng->draw(brick_tex);

                eng->GL_swap_buffers();
            } break;
            default:
                break;
        }
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
