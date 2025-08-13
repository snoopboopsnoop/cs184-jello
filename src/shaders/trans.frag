#version 330 core
in vec3 vPosWS;
in vec3 vPosVS;

out vec4 FragColor;

// lighting params
uniform vec3 lightDirWS = normalize(vec3(0.4, 0.7, 0.2)); // directional light
uniform vec3 lightColor = vec3(1.0);

// jello material controls
uniform vec3 baseColor   = vec3(0.9, 0.1, 0.1); // jelly tint
uniform float alphaFront = 0.35;                 // frontface opacity
uniform float alphaBack  = 0.50;                 // backface opacity (more for thickness)
uniform float wrap       = 0.3;                  // wrap diffuse (0=Lambert, ~0.3..0.6 jelly)
uniform float specPower  = 32.0;                 // specular gloss
uniform float specScale  = 0.4;                  // specular strength
uniform float fresnelF0  = 0.03;                 // ~water/gel

// geometric normal from derivatives (no vertex normals needed)
vec3 geomNormalWS() {
    vec3 dx = dFdx(vPosWS);
    vec3 dy = dFdy(vPosWS);
    vec3 n  = normalize(cross(dx, dy));
    // Make sure normal points to the front face (OpenGL gives us that)
    return gl_FrontFacing ? n : -n;
}

void main() {
    vec3 N = geomNormalWS();
    vec3 L = normalize(lightDirWS);
    vec3 V = normalize(-vPosVS);        // view dir in view space (~ camera at origin)

    // --- wrap diffuse (keeps lit even when light is behind a bit) ---
    float ndl = dot(N, L);
    float diff = clamp((ndl + wrap) / (1.0 + wrap), 0.0, 1.0);

    // --- back lighting (translucency) ---
    float back = max(0.0, dot(-N, L));          // strong when back-lit
    vec3 subsurface = baseColor * pow(back, 1.5); // tweak exponent for softness

    // --- fresnel rim + specular (Blinn-Phong) ---
    vec3 H = normalize(L + V);
    float ndh = max(dot(N, H), 0.0);
    float spec = pow(ndh, specPower) * specScale;

    // Schlick Fresnel
    float ndv = max(dot(N, V), 0.0);
    float fres = fresnelF0 + (1.0 - fresnelF0) * pow(1.0 - ndv, 5.0);

    // combine
    vec3 diffuse = baseColor * diff * lightColor;
    vec3 color = diffuse + subsurface + fres * lightColor * 0.7 + spec * lightColor;

    // two-pass alpha: backfaces a bit more opaque to fake thickness
    float a = gl_FrontFacing ? alphaFront : alphaBack;

    FragColor = vec4(color, a);
}