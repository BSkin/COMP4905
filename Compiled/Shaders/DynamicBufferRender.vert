#version 130

uniform mat4 worldViewProj;
uniform sampler2D texS;
uniform float totalElapsedTime;
uniform vec3 xDir;
uniform vec3 zDir;

attribute vec2 uv;

void main()
{	
	vec4 worldPos = textureLod(texS, uv, 0.0f);

	//snow sway
	float offsetX = 0.1f*sin((totalElapsedTime+17.0f*mod(uv.x*501.0f, 1000.0f))*0.003f);
	float offsetZ = 0.1f*sin((totalElapsedTime+17.0f*mod(uv.y*473.0f, 1000.0f))*0.003f);

	gl_Position = (worldPos.xyz == vec3(0.0f, 0.0f, 0.0f)) ? 
		vec4(0.0f, 0.0f, -1.0f, 0.0f) : worldViewProj * vec4(worldPos.xyz + xDir*offsetX + zDir*offsetZ, 1.0f);	

	gl_PointSize = 5.0f / sqrt(gl_Position.z);
}