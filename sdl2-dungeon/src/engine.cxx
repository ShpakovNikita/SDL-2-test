/*
 * engine.cxx
 *
 *  Created on: 24 нояб. 2017 г.
 *      Author: Shaft
 */
#include "headers/engine.hxx"
#include "headers/picopng.hxx"

#define GLEW_BUILD
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>

#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <windows.h>

const std::string FRAGMENT_PATH = "shaders\\simple_fragment.glsl";
const std::string VERTEX_PATH = "shaders\\simple_vertex.glsl";

#define GL_CHECK()                                                  \
    {                                                               \
        const int err = glGetError();                               \
        if (err != GL_NO_ERROR) {                                   \
            switch (err) {                                          \
                case GL_INVALID_ENUM:                               \
                    std::cerr << GL_INVALID_ENUM << std::endl;      \
                    break;                                          \
                case GL_INVALID_VALUE:                              \
                    std::cerr << GL_INVALID_VALUE << std::endl;     \
                    break;                                          \
                case GL_INVALID_OPERATION:                          \
                    std::cerr << GL_INVALID_OPERATION << std::endl; \
                    break;                                          \
                case GL_INVALID_FRAMEBUFFER_OPERATION:              \
                    std::cerr << GL_INVALID_FRAMEBUFFER_OPERATION   \
                              << std::endl;                         \
                    break;                                          \
                case GL_OUT_OF_MEMORY:                              \
                    std::cerr << GL_OUT_OF_MEMORY << std::endl;     \
                    break;                                          \
            }                                                       \
            assert(false);                                          \
        }                                                           \
    }

namespace CHL {
vertex_2d operator+(vertex_2d& vl, vertex_2d& vr) {
    vertex_2d v;
    v.x = vl.x + vr.x;
    v.y = vl.y + vr.y;
    v.x_t = vl.x_t + vr.x_t;
    v.y_t = vl.y_t + vr.y_t;
    return v;
}

triangle operator+(triangle& tl, triangle& tr) {
    triangle t;
    t.vertices[0] = tl.vertices[0] + tr.vertices[0];
    t.vertices[1] = tl.vertices[1] + tr.vertices[1];
    t.vertices[2] = tl.vertices[2] + tr.vertices[2];
    return t;
}

vertex_2d blend_vertex(const vertex_2d& r, const vertex_2d& l, const float a) {
    vertex_2d v;
    v.x = r.x * (1. - a) + l.x * a;
    v.y = r.y * (1. - a) + l.y * a;
    v.x_t = r.x_t * (1. - a) + l.x_t * a;
    v.y_t = r.y_t * (1. - a) + l.y_t * a;
    return v;
}

triangle blend(const triangle& t1, const triangle& t2, const float a) {
    triangle res;
    res.vertices[0] = blend_vertex(t1.vertices[0], t2.vertices[0], a);
    res.vertices[1] = blend_vertex(t1.vertices[1], t2.vertices[1], a);
    res.vertices[2] = blend_vertex(t1.vertices[2], t2.vertices[2], a);
    return res;
}

struct bind {
    bind(SDL_Keycode k, std::string n, event pressed, event released)
        : key(k), name(n), event_pressed(pressed), event_released(released) {}

    SDL_Keycode key;
    std::string name;
    event event_pressed;
    event event_released;
};

static std::array<std::string, 17> event_names = {
    /// input events
    "left_pressed", "left_released", "right_pressed", "right_released",
    "up_pressed", "up_released", "down_pressed", "down_released",
    "select_pressed", "select_released", "start_pressed", "start_released",
    "button1_pressed", "button1_released", "button2_pressed",
    "button2_released",
    /// virtual console events
    "turn_off"};

const std::array<bind, 8> bindings{
    bind{SDLK_w, "up", event::up_pressed, event::up_released},
    bind{SDLK_a, "left", event::left_pressed, event::left_released},
    bind{SDLK_s, "down", event::down_pressed, event::down_released},
    bind{SDLK_d, "right", event::right_pressed, event::right_released},
    bind{SDLK_LCTRL, "button1", event::button1_pressed,
         event::button1_released},
    bind{SDLK_SPACE, "button2", event::button2_pressed,
         event::button2_released},
    bind{SDLK_ESCAPE, "select", event::select_pressed, event::select_released},
    bind{SDLK_RETURN, "start", event::start_pressed, event::start_released}};

std::istream& operator>>(std::istream& in, vertex_2d& v) {
    in >> v.x;
    in >> v.y;
    in >> v.x_t;
    in >> v.y_t;
    return in;
}

std::istream& operator>>(std::istream& in, triangle& t) {
    in >> t.vertices[0];
    in >> t.vertices[1];
    in >> t.vertices[2];
    return in;
}

std::ostream& operator<<(std::ostream& out, const SDL_version& v) {
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::array<float, 9>& a) {
    for (int i = 0; i < 9; i += 3) {
        out << a[i] << " " << a[i + 1] << std::endl;
    }

    out.flush();
    return out;
}

std::ostream& operator<<(std::ostream& out, triangle& a) {
    for (int i = 0; i < 3; i++) {
        out << a.vertices[i].x << " " << a.vertices[i].y << std::endl;
    }

    out.flush();
    return out;
}

std::ostream& operator<<(std::ostream& stream, const event e) {
    std::uint32_t value = static_cast<std::uint32_t>(e);
    std::uint32_t minimal = static_cast<std::uint32_t>(event::left_pressed);
    std::uint32_t maximal = static_cast<std::uint32_t>(event::turn_off);
    if (value >= minimal && value <= maximal) {    ///По идее вообще это не
                                                   ///должно никогда отрабоатать
                                                   ///так как сюда поступают
                                                   ///лишь уже отсеенные события
        stream << event_names[value];
        return stream;
    } else {
        throw std::runtime_error("too big event value");
    }
}

static bool check_input(const SDL_Event& e, const bind*& result) {
    using namespace std;

    const auto it =
        find_if(bindings.begin(), bindings.end(),
                [&](const bind& b) { return b.key == e.key.keysym.sym; });

    if (it != bindings.end()) {
        result = &(*it);
        return true;
    }
    return false;
}

void convert_triangle(const triangle& t, std::vector<float>& a) {
    for (auto v : t.vertices) {
        a.insert(a.end(), v.x);
        a.insert(a.end(), v.y);
        a.insert(a.end(), 0.f);
        a.insert(a.end(), v.x_t);
        a.insert(a.end(), v.y_t);
    }
}

std::string get_source(const std::string& path) {    // static test!
    std::ifstream source(path);
    std::stringstream stream;

    if (!source.is_open()) {
        throw std::runtime_error("Could not open the file: " + path);
    }

    stream << source.rdbuf();

    return stream.str();
}

engine::engine() {}
engine::~engine() {
    SDL_Quit();
}

instance::instance(std::vector<float> coords, float x, float y, float z) {
    data = coords;
    position = vertex_2d(x, y, 0.0f, 0.0f);
    position.z_index = z;
}

instance::~instance() {
    data.clear();
}

class wall : public instance {
   private:
   public:
    wall(std::vector<float> data, float x, float y, float z_index)
        : instance(data, x, y, z_index) {}

    int render_instance() { return 1; }

    std::vector<float> get_vector() {
        std::vector<float> v;
        v = data;

        for (int i = 0; i < data.size(); i += STRIDE_ELEMENTS) {
            v[i] += position.x;
            v[i + 1] -= position.y;
        }

        return v;
    }
};

instance* create_wall(std::vector<float> data, float x, float y, float z) {
    instance* inst = new wall(data, x, y, z);
    return inst;
}

class player : public instance {
   private:
   public:
    player(std::vector<float> data, float x, float y, float z_index)
        : instance(data, x, y, z_index) {}

    int render_instance() { return 1; }

    std::vector<float> get_vector() {
        std::vector<float> v;
        v = data;

        for (int i = 0; i < data.size(); i++) {
            v[i] += position.x;
            v[i + 1] += position.y;
        }

        return v;
    }
};

bool player_exist = false;

instance* create_player(std::vector<float> data, float x, float y, float z) {
    if (player_exist) {
        throw std::runtime_error("player already exist");
    }

    instance* inst = new player(data, x, y, z);
    player_exist = true;
    return inst;
}

void destroy_player(instance* i) {
    if (player_exist == false) {
        throw std::runtime_error("player is not created");
    }
    if (nullptr == i) {
        throw std::runtime_error("player is null");
    }
    delete i;
}

class engine_impl final : public engine {
   private:
    event_type e_type = event_type::other;
    SDL_Window* window = nullptr;
    GLuint shader_program;

    std::vector<float> vertex_buffer;

    int w_h;
    int w_w;
    int t_size;

    GLuint vao, vbo;

    bool load_texture(std::string path) final {
        std::vector<unsigned char> png_file_in_memory;
        std::ifstream ifs(path.data(), std::ios_base::binary);
        if (!ifs) {
            return false;
        }
        ifs.seekg(0, std::ios_base::end);
        size_t pos_in_file = ifs.tellg();
        png_file_in_memory.resize(pos_in_file);
        ifs.seekg(0, std::ios_base::beg);
        if (!ifs) {
            return false;
        }

        ifs.read(reinterpret_cast<char*>(png_file_in_memory.data()),
                 pos_in_file);
        if (!ifs.good()) {
            return false;
        }

        std::vector<unsigned char> image;
        unsigned long w = 0;
        unsigned long h = 0;
        int error = decodePNG(image, w, h, &png_file_in_memory[0],
                              png_file_in_memory.size(), true);

        // if there's an error, display it
        if (error != 0) {
            std::cerr << "error: " << error << std::endl;
            return false;
        }

        GLuint tex_handl = 0;
        glGenTextures(1, &tex_handl);
        GL_CHECK();
        glBindTexture(GL_TEXTURE_2D, tex_handl);
        GL_CHECK();

        GLint mipmap_level = 0;    // test
        GLint border = 0;

        glTexImage2D(GL_TEXTURE_2D, mipmap_level, GL_RGBA, w, h, border,
                     GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
        GL_CHECK();

        glUniform1i(glGetUniformLocation(shader_program, "our_texture"), 0);
        //        glGenerateMipmap(GL_TEXTURE_2D);
        //        GL_CHECK();

        //        float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        //        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
        //        borderColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return true;
    }

    void GL_unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);    // test
        glBindVertexArray(0);
    }

    GLuint compile_shader(const GLchar* source, GLenum target) {
        GLuint shader = glCreateShader(target);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
        }

        return shader;
    }

    GLuint create_shader_program(GLuint vertex_shader, GLuint fragment_shader) {
        GLuint shader_program = glCreateProgram();

        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);

        glLinkProgram(shader_program);

        GLchar infoLog[512];
        GLint success;

        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(shader_program, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return shader_program;
    }

   public:
    int CHL_init(int width, int height, int size) final {
        SDL_version compiled = {0, 0, 0};
        SDL_version linked = {0, 0, 0};

        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);

        if (SDL_COMPILEDVERSION !=
            SDL_VERSIONNUM(linked.major, linked.minor, linked.patch)) {
            std::cerr << "warning: SDL2 compiled and linked version mismatch: "
                      << compiled << " " << linked << std::endl;
        }

        const int init_result = SDL_Init(SDL_INIT_EVERYTHING);
        if (init_result != 0) {
            const char* err_message = SDL_GetError();
            std::cerr << "error: failed call SDL_Init: " << err_message
                      << std::endl;
            return EXIT_FAILURE;
        }

        window = SDL_CreateWindow("Chlorine-5", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, width, height,
                                  SDL_WINDOW_OPENGL);

        w_h = height;
        w_w = width;

        if (window == nullptr) {
            const char* err_message = SDL_GetError();
            std::cerr << "error: failed call SDL_Init: " << err_message
                      << std::endl;
            SDL_Quit();
            return EXIT_FAILURE;
        }

        SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        assert(gl_context != nullptr);

        GLenum glew_init = glewInit();
        if (glew_init != GLEW_OK) {
            std::cerr << "glewInit error " << glew_init << std::endl;
            SDL_Quit();

            return EXIT_FAILURE;
        }

        int gl_major_ver = 0;
        int res =
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_ver);
        assert(res == 0);

        int gl_minor_ver = 0;
        res = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_minor_ver);
        assert(res == 0);

        std::cerr << "Gl:" << gl_major_ver << '.' << gl_minor_ver << std::endl;

        // test
        glViewport(0, 0, width, height);
        // test

        GLuint vertex_shader =
            compile_shader(get_source(VERTEX_PATH).c_str(), GL_VERTEX_SHADER);

        GLuint fragment_shader = compile_shader(
            get_source(FRAGMENT_PATH).c_str(), GL_FRAGMENT_SHADER);

        shader_program = create_shader_program(vertex_shader, fragment_shader);
        glUseProgram(shader_program);

        if (!load_texture("brick.png")) {
            return EXIT_FAILURE;
        }
        std::cerr << "texture loaded" << std::endl;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDepthFunc(GL_NEVER);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        t_size = size;

        return EXIT_SUCCESS;
    }

    void GL_swap_buffers() final {
        glDrawArrays(GL_TRIANGLES, 0, vertex_buffer.size() / STRIDE_ELEMENTS);
        GL_unbind();
        SDL_GL_SwapWindow(window);

        vertex_buffer.clear();
    }
    void GL_clear_color() final {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    float GL_time() final { return SDL_GetTicks() / 1000.f; }
    event_type get_event_type() final { return e_type; }

    void add_object(const std::vector<float>& data) final {
        for (float f : data)
            vertex_buffer.insert(vertex_buffer.end(), f);
    }

    void draw() final {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     vertex_buffer.size() * sizeof(vertex_buffer[0]),
                     vertex_buffer.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              STRIDE_ELEMENTS * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              STRIDE_ELEMENTS * sizeof(GLfloat),
                              (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        //        GLfloat time = SDL_GetTicks() / 1000.f;
        //        GLfloat red = (sin(time) / 2) + 0.5;
        //        GLint vertexColorLocation =
        //            glGetUniformLocation(shader_program, "our_color");
        //        glUniform4f(vertexColorLocation, red, 0.2f, 0.0f, 1.0f);

        float k = (float)w_w / w_h, scale = 0.5f;
        glm::mat4 transform;
        //        transform = glm::rotate(
        //            transform,
        //            /*(GLfloat)GL_time() * */ glm::radians((float)GL_time()
        //            * 50.f), glm::vec3(0.0f, 0.0f, 1.0f));

        transform = glm::scale(transform,
                               glm::vec3(1.0f * (t_size / (float)w_w),
                                         -1.0f * (t_size / (float)w_h), 1.0f));

        GLint transformLoc = glGetUniformLocation(shader_program, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE,
                           glm::value_ptr(transform));

        GL_CHECK();
    }

    void CHL_exit() final {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return (void)EXIT_SUCCESS;
    }

    bool read_input(event& e) final {
        SDL_Event event;
        e_type = event_type::other;

        if (SDL_PollEvent(&event)) {
            const bind* binding = nullptr;

            switch (event.type) {
                case SDL_QUIT:
                    e = event::turn_off;
                    return true;
                case SDL_KEYDOWN:
                    if (check_input(event, binding)) {
                        e = binding->event_pressed;
                        e_type = event_type::pressed;
                        return true;
                    }
                    break;
                case SDL_KEYUP:
                    if (check_input(event, binding)) {
                        e = binding->event_released;
                        e_type = event_type::released;
                        return true;
                    }
                    break;
                default:
                    break;
            }
        }

        return false;
    }

    ~engine_impl() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

bool already_exist = false;

engine* create_engine() {
    if (already_exist) {
        throw std::runtime_error("engine already exist");
    }
    engine* result = new engine_impl();
    already_exist = true;
    return result;
}

void destroy_engine(engine* e) {
    if (already_exist == false) {
        throw std::runtime_error("Engine is not created");
    }
    if (nullptr == e) {
        throw std::runtime_error("Engine is null");
    }
    delete e;
}

}    // namespace CHL
