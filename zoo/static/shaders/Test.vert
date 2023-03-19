#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform constants {
    vec4 data;
    mat4 render_matrix;
} PushConstants;

void main() {
    gl_Position = PushConstants.render_matrix * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}

