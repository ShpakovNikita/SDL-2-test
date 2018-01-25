/*
 * player.cpp
 *
 *  Created on: 19 янв. 2018 г.
 *      Author: Shaft
 */

#include "headers/player.h"
#include "headers/game_constants.h"
#include "math.h"

#include <iostream>
#include "headers/game_functions.hxx"

CHL::camera* main_camera;

player::player(float x, float y, float z_index, int speed, int size)
    : life_form(x, y, z_index, speed, size) {
    // TODO Auto-generated constructor stub
    health = 10;
    shooting_point = CHL::point(15, -8);
    selected_frame = 0;

    for (int i = 0; i < 18; i++)
        keys[i] = false;

    collision_box.y = TILE_SIZE / 2;
    collision_box.x = TILE_SIZE / 2 + 4;
    frames_in_texture = 12;
    frames_in_animation = 12;
    tilesets_in_texture = 5;
}

player::~player() {
    // TODO Auto-generated destructor stub
}

float change_sprite(player* p) {
    float a = CHL::get_direction(p->mouth_cursor.x, p->mouth_cursor.y,
                                 p->position.x + TILE_SIZE / 2,
                                 p->position.y - TILE_SIZE / 2);

    if (a > (3 * M_PI_2 + M_PI_4) || a <= M_PI_4) {
        p->shooting_point = CHL::point(15, -8);
        if (!p->moving)
            p->selected_frame = 3;
    } else if (a > M_PI_4 && a <= M_PI_2 + M_PI_4) {
        p->shooting_point = CHL::point(p->size.x / 2 + 4, -p->size.x);
        if (!p->moving)
            p->selected_frame = 0;
    } else if (a > M_PI_2 + M_PI_4 && a < M_PI + M_PI_4) {
        p->shooting_point = CHL::point(1, -16);
        if (!p->moving)
            p->selected_frame = 2;
    } else {
        p->shooting_point = CHL::point(p->size.x / 2 - 4, -4);
        if (!p->moving)
            p->selected_frame = 1;
    }
    return a;
}

void player::fire() {
    if (shoot_delay <= 0.0f) {
        manager.get_sound("shot_sound")->play();
        shoot_delay = 0.4f;
        bullets.insert(bullets.end(), new bullet(position.x + shooting_point.x,
                                                 position.y + shooting_point.y,
                                                 0.0f, 4, 0, 2));
        (*(bullets.end() - 1))->alpha =
            calculate_alpha_precision(shooting_alpha);
        (*(bullets.end() - 1))->speed = B_SPEED;
        (*(bullets.end() - 1))->rotation_point =
            CHL::point(position.x + TILE_SIZE / 2, position.y - TILE_SIZE / 2);
        (*(bullets.end() - 1))->creator = bullet_creator::player;
    }
}

void player::super_fire() {
    if (super_delay <= 0.0f) {
        manager.get_sound("shot_sound")->play();
        for (int i = 0; i < 32; i++) {
            bullets.insert(bullets.end(), new bullet(position.x + TILE_SIZE / 2,
                                                     position.y - TILE_SIZE / 2,
                                                     0.0f, 4, 0, 2));
            (*(bullets.end() - 1))->alpha = 2 * M_PI * i / 32.0f;
            (*(bullets.end() - 1))->speed = B_SPEED;
            (*(bullets.end() - 1))->rotation_point = CHL::point(
                position.x + TILE_SIZE / 2, position.y - TILE_SIZE / 2);
            super_delay = 5.0f;
        }
    }
}

void player::move(float dt) {
    float path = speed * dt;
    float a = change_sprite(this);
    shooting_alpha = a;

    position.z_index = position.y;

    delta_x = 0;
    delta_y = 0;

    if (keys[static_cast<int>(CHL::event::up_pressed)]) {
        selected_tileset = 4;
        delta_y = -speed * dt;
    }
    if (keys[static_cast<int>(CHL::event::down_pressed)]) {
        selected_tileset = 3;
        delta_y = speed * dt;
    }
    if (keys[static_cast<int>(CHL::event::left_pressed)]) {
        selected_tileset = 2;
        delta_x = -speed * dt;
    }
    if (keys[static_cast<int>(CHL::event::right_pressed)]) {
        selected_tileset = 1;
        delta_x = speed * dt;
    }

    if (delta_x != 0 && delta_y != 0) {
        delta_x = delta_x / std::sqrt(2);
        delta_y = sign(delta_y) * std::fabs(delta_x);
    }

    if ((delta_x != 0 || delta_y != 0) && !moving) {
        moving = true;
        std::cout << "play anim" << std::endl;
        loop_animation(0.04f);
        manager.get_sound("move_sound")->play_always();
    }

    if (delta_x == 0 && delta_y == 0 && moving) {
        moving = false;
        std::cout << "stop anim" << std::endl;
        loop_animation(0.0f);
        selected_tileset = 0;
        manager.get_sound("move_sound")->stop();
    }

    position.y += delta_y;
    position.x += delta_x;

    if (position.x < 0 || position.x > WINDOW_WIDTH / 2)
        position.x -= delta_x;
    if (position.y - TILE_SIZE < 0 || position.y > WINDOW_HEIGHT / 2)
        position.y -= delta_y;

    if (shoot_delay > 0)
        shoot_delay -= dt;
    if (super_delay > 0)
        super_delay -= dt;

    update_points();
    update();
}
