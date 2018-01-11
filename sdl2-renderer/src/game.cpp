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
#include <chrono>
#include <thread>

#include "headers/engine.hxx"
#include "dungeon.cpp"

#include "headers/bullet.h"

enum class mode { draw, look, idle };

const std::string SND_FOLDER = "sounds\\";

const std::string START_MUSIC = "main.wav";
const std::string IDLE_SOUND = "idle.wav";
const std::string MOVE_SOUND = "move.wav";

const std::string VERTEX_FILE = "vertices.txt";
const std::string SIN_FILE = "sin.txt";

constexpr int WINDOW_WIDTH = 1536;
constexpr int WINDOW_HEIGHT = 960;

constexpr int TILE_SIZE = 16;

constexpr int FPS = 60;

constexpr int P_SPEED = 32;
constexpr int B_SPEED = 100;

template <typename T>
int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE);
    eng->set_virtual_pixel(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);

    int k = -1;

    texture* brick_tex = new texture();
    if (!brick_tex->load_texture("brick.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* player_tex = new texture();
    if (!player_tex->load_texture("hero.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* floor_tex = new texture();
    if (!floor_tex->load_texture("tiles.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* bullet_tex = new texture();
    if (!bullet_tex->load_texture("bullet.png"))
        std::cerr << "Texture not found!" << std::endl;

    std::ifstream fin(VERTEX_FILE);
    assert(!!fin);

    triangle t_arr[2];
    fin >> t_arr[0] >> t_arr[1];
    std::vector<float> data;
    for (auto t : t_arr)
        convert_triangle(t, data);

    mode current_mode = mode::idle;

    bool keys[18];
    for (int i = 0; i < 18; i++)
        keys[i] = false;

    std::vector<instance*> bricks;
    std::vector<bullet*> bullets;

    int x_size = WINDOW_WIDTH / 4 / TILE_SIZE,
        y_size = WINDOW_HEIGHT / 4 / TILE_SIZE;
    DungeonGenerator generator(x_size, y_size);
    auto map = generator.Generate();
    std::vector<int> tile_set = map.Print();

    int X_MAP = WINDOW_WIDTH / TILE_SIZE;
    int Y_MAP = WINDOW_HEIGHT / TILE_SIZE;

    bool placed = false;
    std::unique_ptr<life_form, void (*)(life_form*)> player(
        create_player(data, 0.0f, 7.0f, 0.0f, P_SPEED, TILE_SIZE),
        destroy_player);
    player->collision_box.y = TILE_SIZE / 2;    // /2;

    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            std::cout << *(tile_set.begin() + y * x_size + x) << " ";
            if (*(tile_set.begin() + y * x_size + x) != 0)
                bricks.insert(
                    bricks.end(),
                    create_wall(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE,
                                0.0f, TILE_SIZE));
            else if (!placed && *(tile_set.begin() + y * x_size + x) == 0) {
                std::cout << *(tile_set.begin() + y * x_size + x) << std::endl;
                player->position.x = x * TILE_SIZE;
                player->position.y = y * TILE_SIZE + TILE_SIZE;
                placed = true;
            }
        }
        std::cout << std::endl;
    }

    //    bricks.insert(bricks.end(),
    //                  create_wall(data, 1 * TILE_SIZE, 1 * TILE_SIZE +
    //                  TILE_SIZE,
    //                              0.0f, TILE_SIZE));
    //    bricks.insert(bricks.end(),
    //                  create_wall(data, 1 * TILE_SIZE, 2 * TILE_SIZE +
    //                  TILE_SIZE,
    //                              0.0f, TILE_SIZE));
    //    bricks.insert(bricks.end(),
    //                  create_wall(data, 1 * TILE_SIZE, 3 * TILE_SIZE +
    //                  TILE_SIZE,
    //                              0.0f, TILE_SIZE));

    std::vector<instance*> floor;
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            floor.insert(floor.end(), create_wall(data, x * TILE_SIZE,
                                                  y * TILE_SIZE + TILE_SIZE,
                                                  0.0f, TILE_SIZE));
        }
    }

    sound start_music(SND_FOLDER + START_MUSIC);
    sound idle_sound(SND_FOLDER + IDLE_SOUND);
    sound move_sound(SND_FOLDER + MOVE_SOUND);
    sound shot_sound(SND_FOLDER + "shot.wav");
    idle_sound.play_always();
    //    start_music.play_always();

    bool one_time_change = true;

    //    std::cout << tile_set[tile_set.size() - 1] << " "
    //              << tile_set[tile_set.size() - 2] << std::endl;

    float alpha = 0;

    float delay = 0;

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
                case event::button1_pressed: {
                    for (int i = 0; i < 33; i++) {
                        bullets.insert(
                            bullets.end(),
                            new bullet(data, player->position.x + TILE_SIZE / 2,
                                       player->position.y - 8, 0.0f, 8, 0, 2));
                        (*(bullets.end() - 1))->alpha = 2 * M_PI * i / 32.0f;
                        (*(bullets.end() - 1))->speed = B_SPEED;
                        (*(bullets.end() - 1))->rotation_point =
                            point(player->position.x + TILE_SIZE / 2,
                                  player->position.y - TILE_SIZE / 2);
                    }
                    shot_sound.play();
                } break;
                case event::start_pressed:
                    current_mode = mode::look;
                    break;
                case event::left_mouse_pressed:
                    if (delay <= 0) {
                        bullets.insert(
                            bullets.end(),
                            new bullet(data, player->position.x + TILE_SIZE / 2,
                                       player->position.y - 8, 0.0f, 8, 0, 2));
                        (*(bullets.end() - 1))->alpha = alpha;
                        (*(bullets.end() - 1))->speed = B_SPEED;
                        (*(bullets.end() - 1))->rotation_point =
                            point(player->position.x + TILE_SIZE / 2,
                                  player->position.y - TILE_SIZE / 2);
                        shot_sound.play();
                        delay = 0;
                    }
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

        if (delay > 0)
            delay -= delta_time;

        /*calculate angle*/
        int dx = eng->get_mouse_pos().x - (player->position.x + TILE_SIZE / 2);
        int dy = (player->position.y - TILE_SIZE / 2) - eng->get_mouse_pos().y;

        if (dx >= 0 && dy >= 0)
            alpha = std::atan((float)dy / dx);
        else if (dx >= 0 && dy < 0)
            alpha = std::atan((float)dy / dx) + 2 * M_PI;
        else
            alpha = M_PI + std::atan((float)dy / dx);

        /*smooth moving*/
        bool moved = false;
        float delta_x = 0;
        float delta_y = 0;
        if (keys[static_cast<int>(event::left_pressed)]) {
            moved = true;
            delta_x = -player->speed * delta_time;
        }
        if (keys[static_cast<int>(event::right_pressed)]) {
            moved = true;
            delta_x = player->speed * delta_time;
        }
        if (keys[static_cast<int>(event::up_pressed)]) {
            moved = true;
            delta_y = -player->speed * delta_time;
        }
        if (keys[static_cast<int>(event::down_pressed)]) {
            moved = true;
            delta_y = player->speed * delta_time;
        }

        if (delta_x != 0 && delta_y != 0) {
            delta_x = delta_x / std::sqrt(2);
            delta_y = sign(delta_y) * std::fabs(delta_x);
        } else if (delta_x == 0 && delta_y == 0) {
            /*delta_y = (float)k * 4 / TILE_SIZE;*/
            k *= -1;
        }

        //        std::cout << player->position.x << " " << player->position.y
        //                  << std::endl;

        player->position.x += delta_x;
        player->position.y += delta_y;

        /* play music */
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

        /* check collisions */
        for (instance* inst : bricks) {
            if (check_collision(player.get(), inst)) {
                std::cout << "Collide!" << std::endl;
                player->position.x -= delta_x;
                while (check_collision(player.get(), inst))
                    player->position.y -= delta_y / 4;

                player->position.x += delta_x;
                while (check_collision(player.get(), inst))
                    player->position.x -= delta_x / 4;
            }
        }

        int i = 0;
        for (instance* b : bullets) {
            //        	b->update_points();
            if (b->position.x + TILE_SIZE < 0 ||
                b->position.x > WINDOW_WIDTH / 4 + TILE_SIZE ||
                b->position.y + TILE_SIZE < 0 ||
                b->position.y > WINDOW_HEIGHT / 4 + TILE_SIZE) {
                delete *(bullets.begin() + i);
                bullets.erase(bullets.begin() + i);
                continue;
            }
            for (instance* brick : bricks) {
                if (check_collision(b, brick)) {
                    std::cout << "collide!" << std::endl;
                    delete *(bullets.begin() + i);
                    bullets.erase(bullets.begin() + i);
                    i--;
                    break;
                }
            }
            i++;
        }

        /* draw sprites */
        eng->GL_clear_color();

        for (auto tile : floor)
            eng->add_object(tile);

        if (!floor.empty())
            eng->draw(*floor.begin(), floor_tex);

        for (auto brick : bricks)
            eng->add_object(brick);

        if (!bricks.empty())
            eng->draw(bricks[0], brick_tex);

        i = 0;
        for (auto bullet : bullets) {
            std::cout << "bullet " << i << std::endl;
            i++;
            bullet->move(delta_time);
            eng->add_object(bullet);
        }

        if (!bullets.empty())
            eng->draw(bullets[0], bullet_tex);

        //        if (!bullets.empty())
        //            eng->draw(bullets[0], bullet_tex);

        eng->add_object(player.get());
        eng->draw(player.get(), player_tex);

        eng->GL_swap_buffers();

        /*sleep*/
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
