#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;
layout(location = 0) in vec3 in_position;

void main() {
    out_colour = vec4(
        in_position.r + 0.5,
        in_position.g + 0.5,
        in_position.b + 0.5,
        1.0
    );
}