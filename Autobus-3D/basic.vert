#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec3 inNor;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

out vec3 vPosW;
out vec3 vNorW;
out vec4 vCol;

void main()
{
    vec4 posW = uM * vec4(inPos, 1.0);
    vPosW = posW.xyz;

    mat3 N = mat3(transpose(inverse(uM)));
    vNorW = normalize(N * inNor);

    vCol = inCol;

    gl_Position = uP * uV * posW;
}
