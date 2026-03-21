#version 330 core

in vec3 fragNormal;
in vec3 fragPos;

uniform vec4  ka;
uniform vec4  kd;
uniform vec4  ks;
uniform float shininess;

const vec3 keyLightDir   = normalize(vec3(-1.0, -1.0,  0.0));
const vec3 fillLightDir  = normalize(vec3( 1.0, -0.5,  1.0));
const vec3 backLightDir  = normalize(vec3( 0.0,  1.0,  0.0));

const vec3 keyLightColor  = vec3(1.00, 0.98, 0.94) * 0.4;
const vec3 fillLightColor = vec3(0.59, 0.67, 1.00) * 0.4;
const vec3 backLightColor = vec3(0.78, 0.78, 0.78) * 0.3;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(fragNormal);

    // Use diffuse as base, boost ambient to 80% of diffuse
    // for flat cartoon look matching the rest of your scene
    vec3 ambient = kd.rgb * 0.8;

    vec3 color = ambient;

    float diffKey  = max(dot(normal, -keyLightDir),  0.0);
    color += kd.rgb * keyLightColor * diffKey;

    float diffFill = max(dot(normal, -fillLightDir), 0.0);
    color += kd.rgb * fillLightColor * diffFill;

    float diffBack = max(dot(normal, -backLightDir), 0.0);
    color += kd.rgb * backLightColor * diffBack;

    // Clamp to avoid overexposure
    color = min(color, kd.rgb * 1.2);

    fragColor = vec4(color, kd.a);
}