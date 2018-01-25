/*
 * player.h
 *
 *  Created on: 19 янв. 2018 г.
 *      Author: Shaft
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include "engine.hxx"

class player : public CHL::life_form {
   public:
    player(float x, float y, float z_index, int speed, int size);
    virtual ~player();

    float shooting_alpha = 0;

    friend float change_sprite(player*);
    void move(float dt) override;
    void fire();
    void super_fire();

    CHL::point mouth_cursor;
    CHL::point shooting_point;

    bool keys[18];

    void registrate_keys();

   private:
    float shoot_delay = 0.7f;
    float super_delay = 5.0f;

    bool moving = false;
};

#endif /* PLAYER_H_ */
