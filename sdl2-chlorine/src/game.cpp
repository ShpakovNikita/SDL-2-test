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
#include "headers/special_effect.h"
#include "headers/autotile.hxx"
#include "headers/pathfinders.h"
#include "headers/collision_solves.hxx"

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
constexpr int B_SPEED = 65;

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE, FPS);
    eng->set_virtual_pixel(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);

    int k = -1;

    /* loading textures */
    texture* brick_tex = new texture();
    if (!brick_tex->load_texture("test.png"))
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
    if (!explosion_tex->load_texture("explosion-6.png"))
        std::cerr << "Texture not found!" << std::endl;

    texture* obelisk_tex = new texture();
    if (!obelisk_tex->load_texture("obelisk.png"))
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
    player->weight = 2;

    /* generate dungeon and place character */

    instance*** grid;
    grid = new instance**[y_size];
    for (int i = 0; i < y_size; i++)
        grid[i] = new instance*[x_size];

    int** map_grid;
    map_grid = new int*[y_size];
    for (int i = 0; i < y_size; i++)
        map_grid[i] = new int[x_size];

    point start_p, end_p;
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            grid[y][x] = nullptr;
            map_grid[y][x] = *(tile_set.begin() + y * x_size + x);
            if (*(tile_set.begin() + y * x_size + x) != 0) {
                bricks.insert(
                    bricks.end(),
                    create_wall(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE,
                                1.0f, TILE_SIZE));
                (*(bricks.end() - 1))->frames_in_texture = 13;
                (*(bricks.end() - 1))->tilesets_in_texture = 3;
                (*(bricks.end() - 1))->selected_frame = default_frame;
                (*(bricks.end() - 1))->selected_tileset = default_tileset;
                grid[y][x] = *(bricks.end() - 1);
            } else if (!placed && *(tile_set.begin() + y * x_size + x) == 0) {
                player->position.x = x * TILE_SIZE;
                player->position.y = y * TILE_SIZE + TILE_SIZE;
                placed = true;
                *(tile_set.begin() + y * x_size + x) = 1;
                start_p.x = x;
                start_p.y = y;
            }
        }
    }

    autotile(map_grid, grid, x_size, y_size);

    /* load background */
    std::vector<instance*> floor;
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++) {
            floor.insert(floor.end(), create_wall(data, x * TILE_SIZE,
                                                  y * TILE_SIZE + TILE_SIZE,
                                                  MAX_DEPTH, TILE_SIZE));
            (*(floor.end() - 1))->frames_in_texture = 4;
            (*(floor.end() - 1))->selected_frame = rand() % 4;
        }
    }

    /* place enemies */
    std::vector<enemy*> enemies;

    int count = 0;
    while (count < 5) {
        int x = rand() % x_size;
        int y = rand() % y_size;
        if (*(tile_set.begin() + y * x_size + x) != 1) {
            enemies.insert(
                enemies.end(),
                new enemy(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE, 0.0f,
                          P_SPEED - 25, TILE_SIZE));
            (*(enemies.end() - 1))->frames_in_texture = 4;
            (*(enemies.end() - 1))->collision_box.y = TILE_SIZE / 2;
            end_p.x = x;
            end_p.y = y;
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

    std::vector<special_effect*> se;

    /* pathfinding test */
    int map_grid_pf[x_size * y_size];
    convert2d_array(map_grid, map_grid_pf, x_size, y_size);
    int o_buff[x_size * y_size];
    int s = AStarFindPath(start_p.x, start_p.y, end_p.x, end_p.y, map_grid_pf,
                          x_size, y_size, o_buff, x_size * y_size);
    for (int y = 0; y < y_size; y++) {
        for (int x = 0; x < x_size; x++)
            std::cout << map_grid[y][x] << " ";
        std::cout << std::endl;
    }
    std::cout << start_p.x << " " << start_p.y << std::endl;
    std::cout << end_p.x << " " << end_p.y << std::endl;
    for (int i = 0; i < s; i++)
        std::cout << o_buff[i] / x_size << " "
                  << o_buff[i] - x_size * (o_buff[i] / x_size) << std::endl;

    /* animation test */
    instance* animated_block =
        new instance(data, 5 * TILE_SIZE - 4, 5 * TILE_SIZE + TILE_SIZE - 4,
                     MIN_DEPTH, TILE_SIZE + 8);
    animated_block->frames_in_animation = 11;
    animated_block->frames_in_texture = 11;
    animated_block->loop_animation(0.06f);

    /* running game loop */
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
                    quit = true;
                    break;
                case event::button1_pressed:
                    if (delay <= 0) {
                        for (int i = 0; i < 33; i++) {
                            bullets.insert(
                                bullets.end(),
                                new bullet(data,
                                           player->position.x + TILE_SIZE / 2,
                                           player->position.y - TILE_SIZE / 2,
                                           0.0f, 8, 0, 2));
                            (*(bullets.end() - 1))->alpha =
                                2 * M_PI * i / 32.0f;
                            (*(bullets.end() - 1))->speed = B_SPEED;
                            (*(bullets.end() - 1))->rotation_point =
                                point(player->position.x + TILE_SIZE / 2,
                                      player->position.y - TILE_SIZE / 2);
                            delay = 1;
                        }
                        shot_sound.play();
                    }
                    break;
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
                        delay = 0.7;
                    }
                    break;
                default:
                    break;
            }

            if (eng->get_event_type() == event_type::pressed) {
                keys[static_cast<int>(e)] = true;
            } else {
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
        player->position.z_index = player->position.y;

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
                solve_dynamic_to_static_collision_fast(player.get(), inst,
                                                       delta_x, delta_y);
            }
        }

        if (player->position.x < 0 || player->position.x > WINDOW_WIDTH / 4)
            player->position.x -= delta_x;
        if (player->position.y - TILE_SIZE < 0 ||
            player->position.y > WINDOW_HEIGHT / 4)
            player->position.y -= delta_y;

        /// enemy collision
        for (enemy* e : enemies) {
            e->position.z_index = e->position.y;
            //            if (ray_cast(point(e->position.x, e->position.y),
            //            e->destination,
            //                         bricks))
            //                std::cout << "cast" << std::endl;

            e->move(delta_time);
            e->destination.x = player->position.x + TILE_SIZE / 2;
            e->destination.y = player->position.y - TILE_SIZE / 2;
            for (instance* inst : bricks) {
                if (check_collision(e, inst)) {
                    solve_dynamic_to_static_collision_fast(e, inst, e->delta_x,
                                                           e->delta_y);
                }
            }
            if (check_collision(e, player.get())) {
                //                std::cout << "Collide!" << std::endl;
                solve_dynamic_to_dynamic_collision_fast(
                    player.get(), e, delta_x, delta_y, e->delta_x, e->delta_y);
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
            for (int j = 0; j < enemies.size(); j++) {
                if (check_slow_collision(b, enemies[j], intersection_point)) {
                    delete *(bullets.begin() + i);
                    bullets.erase(bullets.begin() + i);

                    delete *(enemies.begin() + j);
                    enemies.erase(enemies.begin() + j);

                    se.insert(se.end(),
                              new special_effect(
                                  data, intersection_point->x - TILE_SIZE / 2,
                                  intersection_point->y + TILE_SIZE / 2,
                                  MIN_DEPTH, TILE_SIZE));
                    (*(se.end() - 1))->frames_in_texture = 8;

                    i--;
                    goto mark;
                }
            }

            for (instance* brick : bricks) {
                if (check_slow_collision(b, brick, intersection_point)) {
                    delete *(bullets.begin() + i);
                    bullets.erase(bullets.begin() + i);

                    se.insert(se.end(),
                              new special_effect(
                                  data, intersection_point->x - TILE_SIZE / 2,
                                  intersection_point->y + TILE_SIZE / 2,
                                  MIN_DEPTH, TILE_SIZE));
                    (*(se.end() - 1))->frames_in_texture = 8;

                    i--;
                    break;
                }
            }
        mark:
            i++;
        }

        for (int j = 0; j < se.size(); j++) {
            if (se[j]->end()) {
                delete *(se.begin() + j);
                se.erase(se.begin() + j);
            }
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
            eng->draw(*bricks.begin(), brick_tex);

        for (auto bullet : bullets) {
            bullet->move(delta_time);
            eng->add_object(bullet);
        }

        if (!bullets.empty())
            eng->draw(*bullets.begin(), bullet_tex);

        eng->add_object(player.get());
        eng->draw(player.get(), player_tex);

        for (auto enemy : enemies) {
            eng->add_object(enemy);
        }
        if (!enemies.empty())
            eng->draw(*enemies.begin(), player_tex);

        for (auto effect : se) {
            effect->update_frame();
            eng->add_object(effect);
        }

        if (!se.empty())
            eng->draw(*se.begin(), explosion_tex);

        animated_block->update();
        eng->add_object(animated_block);
        eng->draw(animated_block, obelisk_tex);

        eng->GL_swap_buffers();

        /* dynamic sleep */
        float time = (eng->GL_time() - prev_frame) * 1000;
        if (time < 1000 / FPS)
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1000 / FPS - (int)time));
    }

    eng->CHL_exit();
    return EXIT_SUCCESS;
}
