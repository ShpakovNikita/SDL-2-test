/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#define STRIDE_ELEMENTS 5

#include <vector>
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
    vertex_2d() : x(0.f), y(0.f), x_t(0.f), y_t(0.f) {}
    vertex_2d(float _x, float _y, float _x_t, float _y_t)
        : x(_x), y(_y), x_t(_x_t), y_t(_y_t) {}

    float x, y;
    float x_t, y_t;
};

struct triangle {
    triangle() {
        vertices.insert(vertices.end(), vertex_2d());
        vertices.insert(vertices.end(), vertex_2d());
        vertices.insert(vertices.end(), vertex_2d());
    }

    triangle(vertex_2d v1, vertex_2d v2, vertex_2d v3) {
        vertices.insert(vertices.end(), v1);
        vertices.insert(vertices.end(), v2);
        vertices.insert(vertices.end(), v3);
    }

    std::vector<vertex_2d> vertices;
};

std::vector<float> convert_triangle(const triangle&);

std::istream& operator>>(std::istream& in, vertex_2d& v);
std::istream& operator>>(std::istream& in, triangle& t);
triangle operator+(triangle& tl, triangle& tr);

triangle blend(const triangle&, const triangle&, const float);

class engine;

engine* create_engine();
void destroy_engine(engine* e);

enum class event_type { pressed, released, other };

class engine {
   public:
    engine();
    virtual ~engine();

    // sdl package
    virtual void GL_clear_color() = 0;
    virtual void GL_swap_buffers() = 0;
    virtual float GL_time() = 0;

    virtual int CHL_init(int, int) = 0;
    virtual bool read_input(event&) = 0;
    virtual void CHL_exit() = 0;
    virtual void draw_triangle(triangle) = 0;
    virtual bool load_texture(std::string) = 0;
    virtual event_type get_event_type() = 0;
};

std::ostream& operator<<(std::ostream& stream, const event e);

}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
