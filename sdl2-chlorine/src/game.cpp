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
#include "headers/enemy.h"

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

constexpr int FPS = 300;

constexpr int P_SPEED = 32;
constexpr int B_SPEED = 65;

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

    /* loading textures */
    texture* brick_tex = new texture();
    if (!brick_tex->load_texture("brick.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* player_tex = new texture();
    if (!player_tex->load_texture("tank.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* floor_tex = new texture();
    if (!floor_tex->load_texture("tiles.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* bullet_tex = new texture();
    if (!bullet_tex->load_texture("bullet.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* explosion_tex = new texture();
    if (!explosion_tex->load_texture("explosion-1.png"))
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
        create_player(data, 0.0f, 7.0f, 0.0f, P_SPEED, TILE_SIZE - 1),
        destroy_player);
    player->collision_box.y = TILE_SIZE / 2;    // /2;
    player->frames_in_texture = 4;

    /* generate dungeon and place character */
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            std::cout << *(tile_set.begin() + y * x_size + x) << " ";
            if (*(tile_set.begin() + y * x_size + x) != 0) {
                bricks.insert(
                    bricks.end(),
                    create_wall(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE,
                                0.0f, TILE_SIZE));
            } else if (!placed && *(tile_set.begin() + y * x_size + x) == 0) {
                player->position.x = x * TILE_SIZE;
                player->position.y = y * TILE_SIZE + TILE_SIZE;
                placed = true;
            }
        }
        std::cout << std::endl;
    }

    /* load background */
    std::vector<instance*> floor;
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            floor.insert(floor.end(), create_wall(data, x * TILE_SIZE,
                                                  y * TILE_SIZE + TILE_SIZE,
                                                  0.0f, TILE_SIZE));
            (*(floor.end() - 1))->frames_in_texture = 4;
            (*(floor.end() - 1))->selected_frame = rand() % 4;
        }
    }

    /* place enemies */
    std::vector<enemy*> enemies;

    int count = 0;
    while (count < 1) {
        int x = rand() % x_size;
        int y = rand() % y_size;
        if (*(tile_set.begin() + y * x_size + x) != 1) {
            enemies.insert(
                enemies.end(),
                new enemy(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE, 0.0f,
                          P_SPEED - 25, TILE_SIZE));
            (*(enemies.end() - 1))->frames_in_texture = 4;
            (*(enemies.end() - 1))->collision_box.y = TILE_SIZE / 2;
            count++;
        }
    }
    sound start_music(SND_FOLDER + START_MUSIC);
    sound idle_sound(SND_FOLDER + IDLE_SOUND);
    sound move_sound(SND_FOLDER + MOVE_SOUND);
    sound shot_sound(SND_FOLDER + "shot.wav");
    idle_sound.play_always();
    //    start_music.play_always();

    bool one_time_change = true;

    float alpha = 0;

    float delay = 0;

    point shooting_point = point(14, -9);
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
                                       player->position.y - TILE_SIZE / 2, 0.0f,
                                       8, 0, 2));
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
                            new bullet(data,
                                       player->position.x + shooting_point.x,
                                       player->position.y + shooting_point.y,
                                       0.0f, 8, 0, 2));
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
        alpha = get_direction(eng->get_mouse_pos().x, eng->get_mouse_pos().y,
                              player->position.x + TILE_SIZE / 2,
                              player->position.y - TILE_SIZE / 2);

        if (alpha > (3 * M_PI_2 + M_PI_4) || alpha <= M_PI_4) {
            shooting_point = point(15, -8);
            player->selected_frame = 0;
        } else if (alpha > M_PI_4 && alpha <= M_PI_2 + M_PI_4) {
            shooting_point = point(TILE_SIZE / 2 + 4, -TILE_SIZE);
            player->selected_frame = 3;
        } else if (alpha > M_PI_2 + M_PI_4 && alpha < M_PI + M_PI_4) {
            shooting_point = point(1, -16);
            player->selected_frame = 1;
        } else {
            shooting_point = point(TILE_SIZE / 2 - 4, -4);
            player->selected_frame = 2;
        }

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

        /// player collision
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

        if (player->position.x < 0 || player->position.x > WINDOW_WIDTH / 4)
            player->position.x -= delta_x;
        if (player->position.y - TILE_SIZE < 0 ||
            player->position.y > WINDOW_HEIGHT / 4)
            player->position.y -= delta_y;

        /// enemy collision
        for (enemy* e : enemies) {
            e->move(delta_time);
            e->destination.x = player->position.x;
            e->destination.y = player->position.y;
            for (instance* inst : bricks) {
                if (check_collision(e, inst)) {
                    std::cout << "Collide!" << std::endl;
                    e->position.x -= e->delta_x;
                    while (check_collision(e, inst))
                        e->position.y -= e->delta_y / 4;

                    e->position.x += delta_x;
                    while (check_collision(e, inst))
                        e->position.x -= e->delta_x / 4;
                }
            }
            if (check_collision(e, player.get())) {
                std::cout << "Collide!" << std::endl;
                e->position.x -= e->delta_x;
                player->position.x -= delta_x;
                while (check_collision(e, player.get())) {
                    e->position.y -= e->delta_y / 4;
                    player->position.y -= delta_y / 4;
                }

                e->position.x += delta_x;
                player->position.x += delta_x;
                while (check_collision(e, player.get())) {
                    e->position.x -= e->delta_x / 4;
                    player->position.x -= delta_x;
                }
            }
        }

        /// bullets collision
        int i = 0;
        for (instance* b : bullets) {
            if (b->position.x + TILE_SIZE < 0 ||
                b->position.x > WINDOW_WIDTH / 4 + TILE_SIZE ||
                b->position.y + TILE_SIZE < 0 ||
                b->position.y > WINDOW_HEIGHT / 4 + TILE_SIZE) {
                delete *(bullets.begin() + i);
                bullets.erase(bullets.begin() + i);
                continue;
            }

            b->update_points();
            point* intersection_point = new point();
            for (instance* brick : bricks) {
                if (check_slow_collision(b, brick, intersection_point)) {
                    std::cout << "Collide" << std::endl;

                    std::cout << intersection_point->x << " "
                              << intersection_point->y << std::endl;

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

        for (auto bullet : bullets) {
            bullet->move(delta_time);
            eng->add_object(bullet);
        }

        if (!bullets.empty())
            eng->draw(bullets[0], bullet_tex);

        eng->add_object(player.get());
        eng->draw(player.get(), player_tex);

        for (auto enemy : enemies) {
            eng->add_object(enemy);
        }
        if (!enemies.empty())
            eng->draw(enemies[0], player_tex);

        eng->GL_swap_buffers();

        /*sleep*/
        float time = (eng->GL_time() - prev_frame) * 1000;
        if (time < 1000 / FPS)
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1000 / FPS - (int)time));
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
