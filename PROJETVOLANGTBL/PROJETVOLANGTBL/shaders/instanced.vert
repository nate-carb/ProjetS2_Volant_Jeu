#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

// 4 columns of the instance matrix, one vec4 each
in vec4 instanceModelMatrix0;
in vec4 instanceModelMatrix1;
in vec4 instanceModelMatrix2;
in vec4 instanceModelMatrix3;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragNormal;
out vec3 fragPos;

void main()
{
    // Reconstruct the full model matrix from the 4 columns
    mat4 modelMatrix = mat4(
        instanceModelMatrix0,
        instanceModelMatrix1,
        instanceModelMatrix2,
        instanceModelMatrix3
    );

    vec4 worldPos = modelMatrix * vec4(vertexPosition, 1.0);
    gl_Position   = projectionMatrix * viewMatrix * worldPos;

    fragNormal = normalize(mat3(modelMatrix) * vertexNormal);
    fragPos    = worldPos.xyz;
}