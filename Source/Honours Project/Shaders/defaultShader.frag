#version 130

uniform sampler2D texS;

in float lighting;
in vec2 uvVar;
in vec3 normalVar;

out vec4 outColour;

void main()
{
	vec3 sunDir = normalize(vec3(0.1f, 0.5f, 0.5f));
	float intensity = min(1.0f, max(dot(sunDir, normalVar), 0.0f) + 0.2f);

	//outColour = texture(texS, uvVar).rgba;
	//outColour = vec4(intensity*texture(texS, uvVar).rgb, texture(texS, uvVar).a);
	outColour = ((lighting >= 0.5f) ? vec4(intensity*texture(texS, uvVar).rgb, texture(texS, uvVar).a) : texture(texS, uvVar).rgba);
}