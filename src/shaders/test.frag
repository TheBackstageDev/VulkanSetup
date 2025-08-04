#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstant 
{
    mat4 modelMatrix;
    uint textureId;
} push;

#define SUN_DIRECTION vec3(0.0f, 0.5f, -1.0f)
#define AMBIENT_LIGHT vec3(0.1f, 0.1f, 0.1f)

void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0); // Solid red
}