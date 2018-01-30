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

void do_actions(player* p) {
    if (p->keys[p->key_super])
        p->super_fire();
    if (p->keys[p->key_fire])
        p->fire();
    if (p->keys[p->key_blink])
        p->blink();
}

void update_delay(player* p, float dt) {
    if (p->shoot_delay > 0)
        p->shoot_delay -= dt;
    if (p->super_delay > 0)
        p->super_delay -= dt;
    if (p->blink_delay > 0)
        p->blink_delay -= dt;
}

float change_sprite(player* p, change_mode mode) {
    float a = CHL::get_direction(p->mouth_cursor.x, p->mouth_cursor.y,
                                 p->position.x + TILE_SIZE / 2,
                                 p->position.y - TILE_SIZE / 2);

    if (a > (3 * M_PI_2 + M_PI_4) || a <= M_PI_4) {
        p->shooting_point = CHL::point(15, -8);
        if (!p->moving) {
            if (mode == change_mode::blink) {
                p->selected_tileset = 1;
                p->selected_frame = 8;
            } else
                p->selected_frame = 3;
        }
    } else if (a > M_PI_4 && a <= M_PI_2 + M_PI_4) {
        p->shooting_point = CHL::point(p->size.x / 2 + 4, -p->size.x);
        if (!p->moving) {
            if (mode == change_mode::blink) {
                p->selected_tileset = 4;
                p->selected_frame = 8;
            } else
                p->selected_frame = 0;
        }
    } else if (a > M_PI_2 + M_PI_4 && a < M_PI + M_PI_4) {
        p->shooting_point = CHL::point(1, -16);
        if (!p->moving) {
            if (mode == change_mode::blink) {
                p->selected_tileset = 2;
                p->selected_frame = 8;
            } else
                p->selected_frame = 2;
        }
    } else {
        p->shooting_point = CHL::point(p->size.x / 2 - 4, -4);
        if (!p->moving) {
            if (mode == change_mode::blink) {
                p->selected_tileset = 3;
                p->selected_frame = 8;
            } else
                p->selected_frame = 1;
        }
    }
    return a;
}

float check_registred_keys(player* p) {
    float a = -1;

    if (p->keys[p->key_up]) {
        p->selected_tileset = 4;
        a = M_PI_2;
    }
    if (p->keys[p->key_down]) {
        p->selected_tileset = 3;
        a = 3 * M_PI_2;
    }
    if (p->keys[p->key_left]) {
        p->selected_tileset = 2;
        a = M_PI;
    }
    if (p->keys[p->key_right]) {
        p->selected_tileset = 1;
        a = 2 * M_PI;
    }
    if (p->keys[p->key_up] && p->keys[p->key_down]) {
        a = -1;    // mark as not moved. Tileset and frame will choose
                   // automatically
    }
    if (p->keys[p->key_up] && p->keys[p->key_left]) {
        p->selected_tileset = 2;
        a = 3 * M_PI_4;
    }
    if (p->keys[p->key_up] && p->keys[p->key_right]) {
        p->selected_tileset = 1;
        a = M_PI_4;
    }
    if (p->keys[p->key_right] && p->keys[p->key_down]) {
        a = 2 * M_PI - M_PI_4;
    }
    if (p->keys[p->key_left] && p->keys[p->key_down]) {
        a = M_PI + M_PI_4;
    }

    return a;
}

void player::register_keys(CHL::event up,
                           CHL::event down,
                           CHL::event left,
                           CHL::event right,
                           CHL::event fire,
                           CHL::event super,
                           CHL::event blink,
                           CHL::event attack) {
    key_up = static_cast<uint32_t>(up);
    key_down = static_cast<uint32_t>(down);
    key_left = static_cast<uint32_t>(left);
    key_right = static_cast<uint32_t>(right);
    key_fire = static_cast<uint32_t>(fire);
    key_super = static_cast<uint32_t>(super);
    key_blink = static_cast<uint32_t>(blink);
    key_attack = static_cast<uint32_t>(attack);
}

void player::blink() {
    if (blink_delay <= 0 && !blinking) {
        blinking = true;
        blink_delay = 1.0f;
        blinking_path = 32;
        blinking_alpha = change_sprite(this, change_mode::blink);
    }
}

void player::blink_to(const CHL::point& p) {
    if (!blinking) {
        blinking = true;
        blinking_path = CHL::get_distance(position.x, position.y, p.x, p.y);
        blinking_alpha = CHL::get_direction(p.x, p.y, position.x, position.y);
    }
}

void player::fire() {
    if (shoot_delay <= 0.0f && !blinking) {
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
    if (super_delay <= 0.0f && !blinking) {
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
    position.z_index = position.y;

    delta_x = 0;
    delta_y = 0;

    if (blinking) {
        float path = 6 * speed * dt;
        blinking_path -= path;
        delta_x = path * std::cos(blinking_alpha);
        delta_y = -path * std::sin(blinking_alpha);

        position.y += delta_y;
        position.x += delta_x;

        if (blinking_path <= 0) {
            blinking = false;
            selected_tileset = 0;
            change_sprite(this, change_mode::regular);
            return;
        }
        non_material_quads.insert(
            non_material_quads.end(),
            new CHL::instance(position.x, position.y, position.z_index,
                              TILE_SIZE));

        (*(non_material_quads.end() - 1))->frames_in_texture =
            frames_in_texture;
        (*(non_material_quads.end() - 1))->tilesets_in_texture =
            tilesets_in_texture;
        (*(non_material_quads.end() - 1))->selected_tileset = selected_tileset;
        (*(non_material_quads.end() - 1))->selected_frame = selected_frame;

        return;
    }
    float path = speed * dt;
    float a = change_sprite(this, change_mode::regular);
    shooting_alpha = a;
    a = check_registred_keys(this);

    if (a != -1) {
        delta_x = path * std::cos(a);
        delta_y = -path * std::sin(a);
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

    do_actions(this);

    update_delay(this, dt);

    update_points();
    update();
}
