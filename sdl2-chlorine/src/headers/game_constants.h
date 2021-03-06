/*
 * game_constants.h
 *
 *  Created on: 17 янв. 2018 г.
 *      Author: Shaft
 */
#pragma once

#include "engine.hxx"
#include "bullet.h"

#include <string>
#include <vector>
#include <math.h>
#include "resource_manager.h"

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

constexpr int x_size = WINDOW_WIDTH / 4 / TILE_SIZE,
              y_size = WINDOW_HEIGHT / 4 / TILE_SIZE;

extern std::vector<CHL::instance*> bricks;
extern std::vector<bullet*> bullets;
extern std::vector<CHL::life_form*> entities;
extern resource_manager manager;
