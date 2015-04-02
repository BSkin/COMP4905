#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <Windows.h>
#include <string>
#include <stdio.h>

#include "glew.h"
#include <gl/GLU.h>
#include <gl/GL.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"	

#include "Shader.h"
#include "Camera.h"
#include "AssetManager.h"
#include "Settings.h"

class ParticleSystem
{
public:
	ParticleSystem();
	virtual ~ParticleSystem() {}
	virtual void addParticle() {}
	virtual void update(long elapsedTime, long totalElapsedTime, Texture * depthTexture, glm::mat4 * particleMatrix) {}
	virtual void render(long totalElapsedTime) {}

	virtual glm::vec3 getPosition() { return -direction; }
	glm::vec3 getDirection() { return direction; }
	glm::vec3 getUp() { return up; }
	glm::vec3 getTargetPosition() { return glm::vec3(0,0,0); }

	float getSpawnRate() { return spawnRate; }
	void setSpawnRate(float x) { spawnRate = x; if (spawnRate < 0.0f) spawnRate = 0.0f; if (spawnRate > 1000.0f) spawnRate = 1000.0f; }
	void modSpawnRate(float x) { setSpawnRate(spawnRate + x); }
	float getSpeed() { return speed; }
	void setSpeed(float x) { speed = x; if (speed < 0.0f) speed = 0.0f; if (speed > 10.0f) speed = 10.0f; }
	void modSpeed(float x) { setSpeed(speed + x); }
	void updateDirection(glm::vec3 dir);
	void updateDirection(float x, float y, float z) { updateDirection(glm::vec3(x, y, z)); }

	#define activeCamera Camera::getActiveCamera()
	#define activeShader Shader::getActiveShader()

	static void setStatics(AssetManager * x) { assetManager = x; }

protected:
	static AssetManager * assetManager;

	void updateUp();

	glm::vec3 direction, up;
	int numParticles;
	float spawnRate;
	float speed;
};

#endif