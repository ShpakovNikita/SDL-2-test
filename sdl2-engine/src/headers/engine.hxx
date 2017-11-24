/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#include <string>

#include "SDL2/SDL.h"

enum class event {
    /// input events
    left_pressed,
    left_released,
    right_pressed,
    right_released,
    up_pressed,
    up_released,
    down_pressed,
    down_released,
    select_pressed,
    select_released,
    start_pressed,
    start_released,
    button1_pressed,
    button1_released,
    button2_pressed,
    button2_released,
    /// virtual console events
    turn_off
};

struct bind {
    bind(SDL_Keycode k, std::string n, event pressed, event released)
        : key(k), name(n), event_pressed(pressed), event_released(released) {}

    SDL_Keycode key;
    std::string name;
    event event_pressed;
    event event_released;
};

namespace CHL    // chlorine-5
{

class engine {
   public:
    engine();
    virtual ~engine();

    int CHL_init(std::string);

    bool read_input(event&);
    void CHL_exit();

   private:
    SDL_Window* window = nullptr;
};
}    // namespace CHL

std::ostream& operator<<(std::ostream& out, const SDL_version& v);
std::ostream& operator<<(std::ostream& stream, const event e);

#endif /* HEADERS_ENGINE_HXX_ */
