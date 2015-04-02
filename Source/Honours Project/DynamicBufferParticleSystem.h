#ifndef DYNAMIC_BUFFER_PARTICLE_SYSTEM_H
#define DYNAMIC_BUFFER_PARTICLE_SYSTEM_H

#include "ParticleSystem.h"
#include "FrameBuffer.h"
#include "Model.h"
#include <vector>

using std::vector;

class DynamicBufferParticleSystem : public ParticleSystem
{
public:
	DynamicBufferParticleSystem();
	~DynamicBufferParticleSystem();
	virtual void update(long frameRate, long totalElapsedTime, Texture * depthTexture, glm::mat4 * particleMatrix);
	virtual void render(long totalElapsedTime);
	void drawQuad();
	virtual glm::vec3 getPosition();

private:
	FrameBuffer *frameA, *frameB;
	Shader *updateShader, *renderShader;
	GLuint vertexBuffer;

	int width, height;

	bool frameToggle;

	Model * quad;

	void calculateView(glm::mat4 * view, glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector)
	{ *view = glm::lookAt(position, lookAt, upVector); }
	void calculateOrthographicProjection(glm::mat4 * proj, float width, float height, float nearPlane, float farPlane)
	{ *proj = glm::ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, nearPlane, farPlane); }
	void calculateWorld(glm::mat4 * world, glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector)
	{ *world = glm::inverse((glm::lookAt(position, lookAt, upVector))); }
};

#endif