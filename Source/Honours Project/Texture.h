#ifndef TEXTURE_H
#define TEXTURE_H

#include <Windows.h>

#include "glew.h"
#include <gl/GLU.h>
#include <gl/GL.h>
#include "glm/glm.hpp"

#include <string>
#include <stdio.h>

using std::string;

class Texture
{
public:
	Texture();
	~Texture();
	static Texture * loadTexture(string fileName);
	static Texture * loadBlankTexture(GLint width = 512, GLint height = 512);
	static Texture * loadRandomizedTexture(GLint width = 512, GLint height = 512);
	static Texture * loadBlankFloatTexture(GLint width = 512, GLint height = 512);
	static Texture * loadDepthTexture(GLint width = 512, GLint height = 512);
	void bindTexture() { glBindTexture(GL_TEXTURE_2D, textureID); }
	GLuint getTextureID() { return textureID; } 
	glm::vec4 getPixel(float x, float y);
private:
	static Texture * loadErrorTexture();
	static Texture * loadTGA(FILE * file);
	static Texture * loadCompressedTGA(FILE * file);
	static Texture * loadUncompressedTGA(FILE * file);
	static bool generateTexture(Texture * texture, GLenum fileFormat);

	GLubyte * imageData;
	GLuint textureID;
	GLint type;				//GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT
	GLint width, height;	//Image Dimensions
	GLint bitdepth;			//24 or 32
};

#endif 