#version 450

layout(location = 0) out vec4 out_colour;

layout(set = 1, binding = 0) uniform local_uniform_buffer {
    vec4 diffuse_color;
} ui_ubo;

const int SAMP_DIFFUSE = 0;
layout (set = 1, binding = 1) uniform sampler2D samplers[1];

// Data transfer object
layout(location = 1) in struct dto {
    vec2 tex_coord;
} in_dto;

layout(location = 0) flat in int in_mode;

void main() {
    out_colour = ui_ubo.diffuse_color * texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
}