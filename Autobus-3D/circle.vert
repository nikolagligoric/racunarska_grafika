#version 330 core

layout (location = 0) in vec2 aPos;
uniform vec2 uPos;
uniform float uScale;

void main()
{ 
	gl_Position = vec4(aPos * uScale + uPos, 0.0, 1.0); 
}
