/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#define ARRAY_SIZE 9

#include <array>
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

struct vertex_2d {
    vertex_2d() : x(0.f), y(0.f) {}

    float x, y;
};

struct triangle {
    triangle() {
        vertices[0] = vertex_2d();
        vertices[1] = vertex_2d();
        vertices[2] = vertex_2d();
    }

    std::array<vertex_2d, 3> vertices;
};

static std::array<float, ARRAY_SIZE> convert_triangle(const triangle&);

std::istream& operator>>(std::istream& in, vertex_2d& v);
std::istream& operator>>(std::istream& in, triangle& t);

triangle blend(const triangle&, const triangle&, const float);

class engine;

engine* create_engine();
void destroy_engine(engine* e);

class engine {
   public:
    engine();
    virtual ~engine();

    //sdl package
    virtual void GL_clear_color() = 0;
    virtual void GL_swap_buffers() = 0;
    virtual float GL_time() = 0;

    virtual int CHL_init(int, int) = 0;
    virtual bool read_input(event&) = 0;
    virtual void CHL_exit() = 0;
    virtual void draw_triangle(triangle, int) = 0;
};

std::ostream& operator<<(std::ostream& stream, const event e);

}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
