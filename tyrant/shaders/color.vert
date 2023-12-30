#version 460

// layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inNormal;
// layout(location = 2) in vec3 inColor;
// layout(location = 3) in vec2 inUV;
//
// layout(location = 0) out vec3 fragColor;
// layout(location = 1) out vec2 texCoord;
//
// layout(set = 0, binding = 0) uniform CameraBuffer {
// 	mat4 view;
// 	mat4 proj;
// 	mat4 viewproj;
// } cameraData;
//
// struct ObjectData {
//     mat4 model;
// };
//
// layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
//     ObjectData objects[];
// } objectBuffer;
//
//
// // push constants block
// layout( push_constant ) uniform constants {
//     vec4 data;
//     mat4 render_matrix;
// } PushConstants;

// void main() {
//      mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].model;
//      mat4 transformMatrix = (cameraData.viewproj * modelMatrix);
//      gl_Position = transformMatrix * vec4(inPosition, 1.0f);
//      fragColor = inColor;
//      texCoord = inUV;
// }

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}



