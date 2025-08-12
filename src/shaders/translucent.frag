#version 330 core

// inputs from vertex shader
in vec3 vNormal;
in vec3 vLightDir;
in vec3 vHalfVec;

uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform vec3 AmbientColor;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vLightDir);
    vec3 halfVec = normalize(vHalfVec);

    // simple diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);

    // simple specular lighting
    float spec = pow(max(dot(normal, halfVec), 0.0), 32.0);

    // combine ambient, diffuse, and specular
    vec3 color = AmbientColor + DiffuseColor * diff + SpecularColor * spec;

    // angle-based translucency (stronger when backlit)
    float backLit = max(0.0, dot(-normal, lightDir));
    float translucency = mix(0.3, 0.8, backLit);

    FragColor = vec4(color, translucency);
}