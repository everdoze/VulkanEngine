#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

layout(set = 1, binding = 0) uniform local_uniform_buffer {
    vec4 diffuse_color;
} material_ubo;

layout (set = 1, binding = 1) uniform sampler2D diffuse_sampler;

// Data transfer object
layout(location = 1) in struct dto {
    vec2 tex_coord;
} in_dto;

layout(location = 0) flat in int in_mode;

void main() {
    out_colour = material_ubo.diffuse_color * texture(diffuse_sampler, in_dto.tex_coord);
}