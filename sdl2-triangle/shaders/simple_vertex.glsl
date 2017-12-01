#version 330 core

layout (location = 0) in vec3 position;

out vec4 vertex_color;

uniform vec4 our_color;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    vertex_color = our_color;
}
