#version 330 core

in vec3 vPosW;
in vec3 vNorW;
in vec4 vCol;

out vec4 outCol;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

uniform bool transparent;
uniform vec4 uTint;

void main()
{
    vec3 N = normalize(vNorW);
    vec3 L = normalize(uLightPos - vPosW);
    vec3 V = normalize(uViewPos - vPosW);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);

    float spec = 0.0;
    if (diff > 0.0)
        spec = pow(max(dot(V, R), 0.0), transparent ? 16.0 : 32.0);

    vec3 ambient = 0.5 * uLightColor;
    float specK = transparent ? 0.05 : 0.18;

    vec3 lit = (ambient + diff * uLightColor + specK * spec * uLightColor) * vCol.rgb;

    if (transparent)
        outCol = vec4(lit * uTint.rgb, uTint.a);
    else
        outCol = vec4(lit * uTint.rgb, 1.0);

}
