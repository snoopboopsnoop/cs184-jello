#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// light & camera positions in world space
uniform vec3 lightPos;
uniform vec3 eyePos;

// outputs to fragment shader
out vec3 vNormal;
out vec3 vLightDir;
out vec3 vHalfVec;

void main()
{
    // transform vertex to world space
    vec3 worldPos = vec3(model * vec4(position, 1.0));

    // normal in world space
    vNormal = normalize(mat3(model) * normal);

    // light direction vector
    vLightDir = normalize(lightPos - worldPos);

    // view vector eye to vertex
    vec3 viewDir = normalize(eyePos - worldPos);

    // halfway vector
    vHalfVec = normalize(vLightDir + viewDir);

    // final clip-space position
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
