/*
 * collision_solves.hxx
 *
 *  Created on: 17 янв. 2018 г.
 *      Author: Shaft
 */

#ifndef HEADERS_COLLISION_SOLVES_HXX_
#define HEADERS_COLLISION_SOLVES_HXX_

#include "engine.hxx"

template <typename T>
int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

void solve_dynamic_to_dynamic_collision_fast(CHL::instance* one,
                                             CHL::instance* two,
                                             float delta_x1,
                                             float delta_y1,
                                             float delta_x2,
                                             float delta_y2) {
    bool x_col = false, y_col = false;
    one->position.x -= delta_x1;

    if (check_collision(one, two))
        y_col = true;

    one->position.x += delta_x1;
    one->position.y -= delta_y1;

    if (check_collision(one, two))
        x_col = true;

    one->position.x -= delta_x1;

    if (!y_col)
        one->position.y += delta_y1;
    if (!x_col)
        one->position.x += delta_x1;

    while (y_col) {
        one->position.y += sign(delta_y1) * 0.02;
        two->position.y += sign(delta_y2) * 0.05;
        if (check_collision(one, two)) {
            one->position.y -= sign(delta_y1) * 0.02;
            two->position.y -= sign(delta_y2) * 0.05;
            y_col = false;
        }
    }

    while (x_col) {
        one->position.x += sign(delta_x1) * 0.02;
        two->position.x += sign(delta_x2) * 0.05;
        if (check_collision(one, two)) {
            one->position.x -= sign(delta_x1) * 0.02;
            two->position.x -= sign(delta_x2) * 0.05;
            x_col = false;
        }
    }
}

void solve_dynamic_to_static_collision_fast(CHL::instance* dyn,
                                            CHL::instance* inst,
                                            float delta_x,
                                            float delta_y) {
    bool x_col = false, y_col = false;
    dyn->position.x -= delta_x;

    if (check_collision(dyn, inst))
        y_col = true;

    dyn->position.x += delta_x;
    dyn->position.y -= delta_y;

    if (check_collision(dyn, inst))
        x_col = true;

    dyn->position.x -= delta_x;

    if (!y_col)
        dyn->position.y += delta_y;
    if (!x_col)
        dyn->position.x += delta_x;

    while (y_col) {
        dyn->position.y += sign(delta_y) * 0.05;
        if (check_collision(dyn, inst)) {
            dyn->position.y -= sign(delta_y) * 0.05;
            y_col = false;
        }
    }

    while (x_col) {
        dyn->position.x += sign(delta_x) * 0.05;
        if (check_collision(dyn, inst)) {
            dyn->position.x -= sign(delta_x) * 0.05;
            x_col = false;
        }
    }
}

#endif /* HEADERS_COLLISION_SOLVES_HXX_ */
