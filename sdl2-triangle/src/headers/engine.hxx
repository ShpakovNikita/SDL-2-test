/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#include <string>

namespace CHL    // chlorine-5
{
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

class engine;

engine* create_engine();
void destroy_engine(engine* e);

class engine {
   public:
    engine();
    virtual ~engine();

    virtual int CHL_init(int, int) = 0;
    virtual bool read_input(event&) = 0;
    virtual void CHL_exit() = 0;
};

std::ostream& operator<<(std::ostream& stream, const event e);

}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
