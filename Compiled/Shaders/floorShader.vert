#version 130

uniform mat4 worldViewProj;
uniform float enableLighting;
uniform mat4 normalMatrix;
uniform mat4 worldMatrix;

attribute vec3 vertex;
attribute vec2 uv;
attribute vec3 normal;

out float lighting;
out vec2 uvVar;
out vec3 normalVar;
out vec3 worldPos;

void main()
{
	lighting = enableLighting;
	uvVar = uv;
	worldPos = (worldMatrix * (vec4(vertex, 1.0f))).xyz;
	gl_Position = worldViewProj * vec4(vertex, 1.0f);
	normalVar = vec3(normalMatrix * vec4(normal, 0.0f));
}