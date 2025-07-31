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

#define SUN_DIRECTION vec3(0.0f, 0.5f, 0.2f)
#define AMBIENT_LIGHT vec3(0.1f, 0.1f, 0.1f)

void main()
{
    vec3 lightDirection = normalize(SUN_DIRECTION);
    float diffuse = max(dot(normal, lightDirection), 0.0f);
    
    vec4 texColor = texture(textures[push.textureId], uv);
    outColor = vec4(texColor.rgb * color.rgb * (AMBIENT_LIGHT + diffuse), color.a);
}
