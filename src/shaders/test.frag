#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

#define SUN_DIRECTION vec3(0.0f, 0.5f, 0.2f)
#define AMBIENT_LIGHT vec3(0.1f, 0.1f, 0.1f)

void main()
{
    vec3 lightDirection = normalize(SUN_DIRECTION);
    float diffuse = max(dot(normal, lightDirection), 0.0f);
    
    outColor = vec4(fragColor.rgb * (AMBIENT_LIGHT + diffuse), fragColor.a);
}
