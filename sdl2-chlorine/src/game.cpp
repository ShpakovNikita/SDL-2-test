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
#include "headers/player.h"

#include "headers/game_constants.h"

enum class mode { draw, look, idle };

int main(int /*argc*/, char* /*argv*/ []) {
    using namespace CHL;
    std::unique_ptr<engine, void (*)(engine*)> eng(create_engine(),
                                                   destroy_engine);

    eng->CHL_init(WINDOW_WIDTH, WINDOW_HEIGHT, TILE_SIZE, FPS);
    eng->set_virtual_pixel(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);

    eng->GL_clear_color();
    eng->GL_swap_buffers();

    /* loading textures */

    manager.add_texture("brick", new texture("test.png"));
    manager.add_texture("player", new texture("tank.png"));
    manager.add_texture("floor", new texture("tiles.png"));
    manager.add_texture("bullet", new texture("bullet.png"));
    manager.add_texture("explosion", new texture("explosion-6.png"));
    manager.add_texture("obelisk", new texture("obelisk.png"));

    std::ifstream fin(VERTEX_FILE);
    assert(!!fin);

    triangle t_arr[2];
    fin >> t_arr[0] >> t_arr[1];
    std::vector<float> data;
    for (auto t : t_arr)
        convert_triangle(t, data);

    DungeonGenerator generator(x_size, y_size);
    auto map = generator.Generate();
    std::vector<int> tile_set = map.Print();

    int X_MAP = WINDOW_WIDTH / TILE_SIZE;
    int Y_MAP = WINDOW_HEIGHT / TILE_SIZE;

    bool placed = false;
    player* hero = new player(data, 0.0f, 7.0f, 0.0f, P_SPEED, TILE_SIZE);
    hero->collision_box.y = TILE_SIZE / 2;    // /2;
    hero->frames_in_texture = 4;
    hero->weight = 2;

    entities.insert(entities.end(), hero);
    /* generate dungeon and place character */

    instance*** grid;
    grid = new instance**[y_size];
    for (int i = 0; i < y_size; i++)
        grid[i] = new instance*[x_size];

    int** map_grid;
    map_grid = new int*[y_size];
    for (int i = 0; i < y_size; i++)
        map_grid[i] = new int[x_size];

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
                hero->position.x = x * TILE_SIZE;
                hero->position.y = y * TILE_SIZE + TILE_SIZE;
                placed = true;
                *(tile_set.begin() + y * x_size + x) = 1;
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

    int map_grid_pf[x_size * y_size];
    int count = 0;
    while (count < 1) {
        int x = rand() % x_size;
        int y = rand() % y_size;
        if (*(tile_set.begin() + y * x_size + x) != 1) {
            entities.insert(
                entities.end(),
                new enemy(data, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE, 0.0f,
                          P_SPEED - 17, TILE_SIZE));
            (*(entities.end() - 1))->frames_in_texture = 4;
            (*(entities.end() - 1))->collision_box.y = TILE_SIZE / 2;
            dynamic_cast<enemy*>(*(entities.end() - 1))->map = map_grid_pf;
            dynamic_cast<enemy*>(*(entities.end() - 1))->destination.x =
                hero->position.x;
            dynamic_cast<enemy*>(*(entities.end() - 1))->destination.y =
                hero->position.y;
            *(tile_set.begin() + y * x_size + x) = 1;
            count++;
        }
    }
    manager.add_sound("start_music", new sound(SND_FOLDER + START_MUSIC));
    manager.add_sound("idle_sound", new sound(SND_FOLDER + IDLE_SOUND));
    manager.add_sound("move_sound", new sound(SND_FOLDER + MOVE_SOUND));
    manager.add_sound("shot_sound", new sound(SND_FOLDER + "shot.wav"));
    manager.get_sound("idle_sound")->play_always();
    manager.get_sound("start_music")->play_always();

    point shooting_point = point(14, -9);
    float prev_frame = eng->GL_time();
    bool quit = false;

    std::vector<special_effect*> se;

    convert2d_array(map_grid, map_grid_pf, x_size, y_size);

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
                    hero->super_fire();
                    break;

                case event::left_mouse_pressed:
                    hero->fire();
                    break;
                default:
                    break;
            }

            if (eng->get_event_type() == event_type::pressed) {
                hero->keys[static_cast<int>(e)] = true;
            } else {
                hero->keys[static_cast<int>(e) - 1] = false;
            }
        }
        hero->move(delta_time);

        /*calculate angle*/
        hero->mouth_cursor.x = eng->get_mouse_pos().x;
        hero->mouth_cursor.y = eng->get_mouse_pos().y;

        /* check collisions */

        /// player & enemy collision
        int tst = 0;
        for (life_form* lf : entities) {
            lf->position.z_index = lf->position.y;
            enemy* e = dynamic_cast<enemy*>(lf);
            if (e != nullptr) {
                e->move(delta_time);
                e->destination.x = hero->position.x + TILE_SIZE / 2;
                e->destination.y = hero->position.y - TILE_SIZE / 4;
            }
            for (instance* inst : bricks) {
                if (check_collision(lf, inst)) {
                    solve_dynamic_to_static_collision_fast(
                        lf, inst, lf->delta_x, lf->delta_y);
                }
            }
            tst++;
        }

        int i = 0;
        hero->update_points();
        for (bullet* b : bullets) {
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

            bool smth_destroyed = false;
            for (int j = 0; j < entities.size(); j++) {
                if (check_slow_collision(b, entities[j], intersection_point)) {
                    if (dynamic_cast<enemy*>(entities[j]) != nullptr &&
                        b->creator != bullet_creator::enemy) {
                        delete *(bullets.begin() + i);
                        bullets.erase(bullets.begin() + i);

                        if (--entities[j]->health <= 0) {
                            delete *(entities.begin() + j);
                            entities.erase(entities.begin() + j);
                        }

                        se.insert(
                            se.end(),
                            new special_effect(
                                data, intersection_point->x - TILE_SIZE / 2,
                                intersection_point->y + TILE_SIZE / 2,
                                MIN_DEPTH, TILE_SIZE));
                        (*(se.end() - 1))->frames_in_texture = 8;
                        j = entities.size();
                        smth_destroyed = true;
                        i--;
                    } else if (dynamic_cast<player*>(entities[j]) != nullptr &&
                               b->creator == bullet_creator::enemy) {
                        delete *(bullets.begin() + i);
                        bullets.erase(bullets.begin() + i);
                        i--;

                        se.insert(
                            se.end(),
                            new special_effect(
                                data, intersection_point->x - TILE_SIZE / 2,
                                intersection_point->y + TILE_SIZE / 2,
                                MIN_DEPTH, TILE_SIZE));
                        (*(se.end() - 1))->frames_in_texture = 8;
                        if (--entities[j]->health <= 0) {
                            quit = true;
                        }
                        j = entities.size();
                        smth_destroyed = true;
                    }
                }
            }

            if (smth_destroyed)
                continue;

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
            eng->draw(manager.get_texture("floor"));

        for (auto brick : bricks)
            eng->add_object(brick);

        if (!bricks.empty())
            eng->draw(manager.get_texture("brick"));

        for (auto bullet : bullets) {
            bullet->move(delta_time);
            eng->add_object(bullet);
        }

        if (!bullets.empty())
            eng->draw(manager.get_texture("bullet"));

        eng->add_object(hero);
        eng->draw(manager.get_texture("player"));

        for (auto e : entities) {
            if (dynamic_cast<enemy*>(e) != nullptr)
                eng->add_object(e);
        }
        if (!entities.empty())
            eng->draw(manager.get_texture("player"));

        for (auto effect : se) {
            effect->update_frame();
            eng->add_object(effect);
        }

        if (!se.empty())
            eng->draw(manager.get_texture("explosion"));

        animated_block->update();
        eng->add_object(animated_block);
        eng->draw(manager.get_texture("obelisk"));

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
