/*
 * engine.hxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */

#ifndef HEADERS_ENGINE_HXX_
#define HEADERS_ENGINE_HXX_

#define GLEW_BUILD

#define MAX_DEPTH -10000
#define MIN_DEPTH 10000
#define STRIDE_ELEMENTS 5

#include <vector>
#include <string>
#include <memory>
#include <map>

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
    /// mouse events
    left_mouse_pressed,
    /// virtual console events
    turn_off
};

struct point {
    point(int _x, int _y) : x(_x), y(_y) {}
    point() : x(0.0f), y(0.0f) {}
    float x, y;
};

struct vertex_2d {
    vertex_2d() : x(0.f), y(0.f), z_index(0), x_t(0.f), y_t(0.f) {}
    vertex_2d(float _x, float _y, float _x_t, float _y_t)
        : x(_x), y(_y), z_index(0), x_t(_x_t), y_t(_y_t) {}

    float x, y;
    int z_index;
    float x_t, y_t;
};

struct vec3 {
    vec3(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    float x, y, z;
};

struct character {
    unsigned int texture_id;
    point size;
    point bearing;
    unsigned int advance;
};

class font {
   public:
    font(std::string path);
    virtual ~font();
    std::map<char, character> characters;
};

class instance;
class engine;

engine* create_engine();
void destroy_engine(engine* e);

enum class event_type { pressed, released, other };

bool check_collision(instance*, instance*);
bool check_slow_collision(instance* one, instance* two, point*);

float get_direction(float x1, float y1, float x2, float y2);
float get_distance(float x1, float y1, float x2, float y2);
bool ray_cast(const point&, const point&, const std::vector<instance*>& map);
bool ray_cast(instance*, const point&, const std::vector<instance*>& map);
bool ray_cast(instance*, instance*, const std::vector<instance*>& map);

class texture {
   public:
    texture(const std::string&);
    texture();
    ~texture();

    bool load_texture(const std::string&);

    void bind();
    void unbind();

    float alpha = 1.0f;

   private:
    int w;
    int h;
    unsigned int tex;
};

class camera {
   public:
    int width, height;
    int border_width, border_height;
    instance* bind_object;

    void update_center();
    point get_center();

    camera(int w, int h, int b_w, int b_h, instance*);
    virtual ~camera();

   private:
    point center;
};

class instance {
   public:
    instance(float x, float y, float z, int size);
    instance(float x, float y, float z, int size_x, int size_y);
    virtual ~instance();

    vertex_2d position;
    point size;

    float weight = 1.0f;

    float alpha = 0;
    point rotation_point = point(0, 0);
    point collision_box;
    int selected_frame = 0;
    int selected_tileset = 0;
    int frames_in_texture = 1;
    int tilesets_in_texture = 1;
    int frames_in_animation = 1;    //!

    std::array<point, 4> mesh_points;
    std::string tag = "none";
    float alpha_channel = 1.0f;

    void get_points();

    void update_points();
    std::vector<float> get_vector();

    void update();    // !
    void play_animation(float seconds_betweeen_frames,
                        int tileset);    // !
    void loop_animation(float seconds_betweeen_frames,
                        int tileset);                      //!
    void play_animation(float seconds_betweeen_frames);    // !
    void loop_animation(float seconds_betweeen_frames);

   protected:
    bool animation_playing = false;    //!
    bool animation_loop = false;       //!
    int delay;                         //!
    float delta_frame;

   private:
    int prev_tileset;
};

class life_form : public instance {
   public:
    life_form(float x, float y, float z, int _speed, int s);
    virtual ~life_form();
    virtual void move(float) = 0;
    int speed;
    int health = 1;
    float delta_x = 0, delta_y = 0;
};

instance* create_wall(float x, float y, float z, int size);

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

class user_interface {
   public:
    user_interface(camera*);
    virtual ~user_interface();
    void add_instance();

   private:
    camera* focus_camera;
    std::vector<instance*> user_interface_elements;
};

class engine {
   public:
    engine();
    virtual ~engine();

    // sdl package
    virtual void GL_clear_color() = 0;
    virtual void GL_swap_buffers() = 0;
    virtual float GL_time() = 0;

    virtual int CHL_init(int, int, int, int) = 0;
    virtual bool read_input(event&) = 0;
    virtual void CHL_exit() = 0;
    virtual point get_mouse_pos(camera*) = 0;
    virtual void add_object(instance*, camera*) = 0;
    virtual void draw(texture*, camera*, instance*) = 0;
    virtual void render_text(const std::string& text,
                             font* f,
                             float x,
                             float y,
                             float offset,
                             int z_pos,
                             vec3 color) = 0;
    virtual void set_virtual_pixel(int, int) = 0;
    //    virtual bool load_texture(std::string) = 0;
    virtual event_type get_event_type() = 0;
};

std::ostream& operator<<(std::ostream& stream, const event e);

}    // namespace CHL

#endif /* HEADERS_ENGINE_HXX_ */
