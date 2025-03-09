#version 450

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform local_uniform_object {
    vec4 diffuse_color;
    float shininess;
} object_ubo;

struct directional_light {
    vec3 direction;
    vec4 color;
};

struct point_light {
    vec3 position;
    vec4 color;
    // Usually 1, make sure denominator never gets smaller than 1
    float constant;
    // Reduces light intensity linearly
    float linear;
    // Makes the light fall off slower at longer distances.
    float quadratic;
};

vec4 ambient_color = vec4(0.25, 0.25, 0.25, 1);

// TODO: feed in from cpu
directional_light dir_light = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.6, 0.6, 0.6, 1.0)
};

// TODO: feed in from cpu
point_light p_light_0 = {
    vec3(-5.5, 0.0, -5.5),
    vec4(1.0, 1.0, 0.0, 1.0),
    1.0, // Constant
    0.35, // Linear
    0.74  // Quadratic
};

// TODO: feed in from cpu
point_light p_light_1 = {
    vec3(5.5, 0.0, 5.5),
    vec4(0.0, 1.0, 1.0, 1.0),
    1.0, // Constant
    0.35, // Linear
    0.74  // Quadratic
};

// Samplers, diffuse, spec
const int SAMP_DIFFUSE = 0;
const int SAMP_SPECULAR = 1;
const int SAMP_NORMAL = 2;
layout(set = 1, binding = 1) uniform sampler2D samplers[3];

layout(location = 0) flat in int in_mode;
// Data Transfer Object
layout(location = 1) in struct dto {
    vec4 ambient;
	vec2 tex_coord;
	vec3 normal;
	vec3 view_position;
	vec3 frag_position;
    vec4 color;
	vec4 tangent;
} in_dto;

mat3 TBN;

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction);
vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction);

// vec4 calculate_simple_lighting(vec3 normal, vec3 view_direction) {
//     vec4 light_color = vec4(1.0, 1.0, 1.0, 1.0); // Simplified white light
//     vec3 light_direction = normalize(vec3(-0.57735, -0.57735, -0.57735));

//     float diffuse_factor = max(dot(normal, light_direction), 0.0);

//     vec4 diff_samp = texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
//     vec4 ambient = vec4(vec3(ambient_color * vec4(1.0, 1.0, 1.0, 1.0)), diff_samp.a);
//     vec4 diffuse = vec4(vec3(light_color * diffuse_factor), diff_samp.a);
//     //vec4 specular = vec4(vec3(light_color * specular_factor), diff_samp.a);
    
//     if(in_mode == 0 || in_mode == 2) {
//         diffuse *= diff_samp;
//         ambient *= diff_samp;
//         //specular *= diff_samp;
//     }

//     return vec4(ambient.rgb + diffuse.rgb, diffuse.a);

//     return vec4(vec3(light_color * diffuse_factor), 1.0);
// }

vec3 calculate_normal() {
    vec3 normal = normalize(in_dto.normal);
    vec3 tangent = normalize(in_dto.tangent.xyz);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = normalize(cross(normal, tangent) * in_dto.tangent.w);

    mat3 TBN = mat3(tangent, bitangent, normal);
    vec3 texture_normal = texture(samplers[SAMP_NORMAL], in_dto.tex_coord).rgb;
    texture_normal = normalize(texture_normal * 2.0 - 1.0);

    return normalize(TBN * texture_normal);
}

void main() {
    vec3 normal = calculate_normal();

    vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);

    out_color = calculate_directional_light(dir_light, normal, view_direction);

    out_color += calculate_point_light(p_light_0, normal, in_dto.frag_position, view_direction);
    out_color += calculate_point_light(p_light_1, normal, in_dto.frag_position, view_direction);
} 

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction) {
    float diffuse_factor = max(dot(normal, -light.direction), 0.0);

    vec3 half_direction = normalize(view_direction - light.direction);
    float specular_factor = pow(max(dot(half_direction, normal), 0.0), object_ubo.shininess);

    vec4 diff_samp = texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
    vec4 ambient = vec4(vec3(ambient_color * object_ubo.diffuse_color), diff_samp.a);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), diff_samp.a);
    vec4 specular = vec4(vec3(light.color * specular_factor), diff_samp.a);
    
    if(in_mode == 0 || in_mode == 2) {
        diffuse *= diff_samp;
        ambient *= diff_samp;
        specular *= diff_samp;
    }

    return vec4(ambient.rgb + diffuse.rgb + specular.rgb, diffuse.a);
}

vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction) {
    vec3 light_direction =  normalize(light.position - frag_position);
    float diff = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), object_ubo.shininess);

    // Calculate attenuation, or light falloff over distance.
    float dist = length(light.position - frag_position);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    vec4 ambient = in_dto.ambient;
    vec4 diffuse = light.color * diff;
    vec4 specular = light.color * spec;
    
    if(in_mode == 0) {
        vec4 diff_samp = texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
        diffuse *= diff_samp;
        ambient *= diff_samp;
        specular *= vec4(texture(samplers[SAMP_SPECULAR], in_dto.tex_coord).rgb, diffuse.a);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}