layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec4 vertex_color;
out vec2 tex_coord;

uniform vec4 our_color;
uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(position, 1.0);
    vertex_color = our_color;
    tex_coord = texCoord;
}
