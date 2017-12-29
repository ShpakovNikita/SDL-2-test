/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#define GLEW_BUILD

#define STRIDE_ELEMENTS 5

#include <vector>
#include <string>
#include <memory>

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
    vertex_2d() : x(0.f), y(0.f), z_index(0.f), x_t(0.f), y_t(0.f) {}
    vertex_2d(float _x, float _y, float _x_t, float _y_t)
        : x(_x), y(_y), z_index(0.f), x_t(_x_t), y_t(_y_t) {}

    float x, y, z_index;
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

void convert_triangle(const triangle&, std::vector<float>&);

std::istream& operator>>(std::istream& in, vertex_2d& v);
std::istream& operator>>(std::istream& in, triangle& t);
triangle operator+(triangle& tl, triangle& tr);

triangle blend(const triangle&, const triangle&, const float);

class instance;
class engine;

engine* create_engine();
void destroy_engine(engine* e);

enum class event_type { pressed, released, other };

bool check_collision(instance&, instance&);

class texture {
   public:
    texture();
    ~texture();

    bool load_texture(const std::string&);

    void bind();
    void unbind();

   private:
    int w;
    int h;
    unsigned int tex;
};

class instance {
   public:
    virtual int render_instance() = 0;
    instance(std::vector<float>, float x, float y, float z);
    virtual ~instance();

    vertex_2d position;
    vertex_2d size;

    virtual std::vector<float> get_vector() = 0;

   protected:
    std::vector<float> data;
};

class life_form : public instance {
   public:
    life_form(std::vector<float>, float x, float y, float z, int speed);
    virtual ~life_form();
    virtual void move() = 0;
};
life_form* create_player(std::vector<float>, float x, float y, float z);
void destroy_player(life_form*);

instance* create_wall(std::vector<float> data, float x, float y, float z);

class sound {
   public:
    sound(const std::string&);
    bool load(const std::string&);
    void play() const;
    void play_always() const;
    void stop() const;
    ~sound();

   private:
    uint32_t device_id;
    uint8_t* buffer;
    uint32_t buffer_size;
};

class engine {
   public:
    engine();
    virtual ~engine();

    // sdl package
    virtual void GL_clear_color() = 0;
    virtual void GL_swap_buffers() = 0;
    virtual float GL_time() = 0;

    virtual int CHL_init(int, int, int) = 0;
    virtual bool read_input(event&) = 0;
    virtual void CHL_exit() = 0;
    virtual void add_object(const std::vector<float>&) = 0;
    virtual void draw() = 0;
    //    virtual bool load_texture(std::string) = 0;
    virtual event_type get_event_type() = 0;
};

std::ostream& operator<<(std::ostream& stream, const event e);

}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
