out vec4 color;

in vec4 vertex_color;
in vec2 tex_coord;

uniform sampler2D our_texture;

void main()
{
    color = texture(our_texture, tex_coord);
}