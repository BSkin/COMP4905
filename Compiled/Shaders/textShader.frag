#version 150

uniform sampler2D texS;
uniform float screenWidth;
uniform float screenHeight;

in vec2 uvVar;
out vec4 outColour;

void main()
{
	outColour = texture(texS, uvVar + vec2(0.5f/screenWidth, 0.5f/screenHeight));
}