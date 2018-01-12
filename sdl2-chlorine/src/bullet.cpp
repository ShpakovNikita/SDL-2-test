/*
 * bullet.cpp
 *
 *  Created on: 6 янв. 2018 г.
 *      Author: Shaft
 */

#include "headers/bullet.h"
#include <iostream>

#include <math.h>

bullet::bullet(std::vector<float> data,
               float x,
               float y,
               float z,
               int _size,
               int _damage,
               float _alpha)
    : instance(data, x, y, z, _size) {
    damage = _damage;
    alpha = _alpha;
    speed = 1;
}

bullet::~bullet() {
    std::cerr << "Destroyed bullet!" << std::endl;
}

void bullet::move(float dt) {
    float path = speed * dt;

    position.y -= path * std::sin(alpha);
    position.x += path * std::cos(alpha);
}
