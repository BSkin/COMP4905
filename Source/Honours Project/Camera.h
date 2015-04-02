#ifndef CAMERA_H
#define CAMERA_H

#include <Windows.h>
#include <math.h>
#include <gl/GLU.h>
#include <gl/GL.h>

#include "glm/glm.hpp"						//glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include "glm/gtc/matrix_transform.hpp"		//glm::translate, glm::rotate, glm::scale
#include "glm/gtc/type_ptr.hpp"				//glm::make_mat4
#include "glm/gtx/rotate_vector.hpp"

#include "Settings.h"

class Camera
{
public:
	Camera();
	~Camera();
	static Camera * getActiveCamera() { return activeCamera; }
	int activate();

	int calculateView();
	int calculateProjection();
	int calculateOrthographicProjection();

	const glm::mat4 * getProjection() { return &projection; }
	const glm::mat4 * getView() { return &view; }

	void setvFOV(float x) { vFOV = x; }
	void setAspectRatio(float x) { aspectRatio = x; }
	void setNearPlane(float x) { nearPlane = x; }
	void setFarPlane(float x) { farPlane = x; }
	void setPerpectiveVars(float vFOV, float apectRatio, float nearPlane, float farPlane) 
	{ setvFOV(vFOV); setAspectRatio(aspectRatio); setNearPlane(nearPlane); setFarPlane(farPlane); }

	void setOrthoWidth(float x) { orthoWidth = x; }
	void setOrthoHeight(float x) { orthoHeight = x; }
	void setOrthoNear(float x) { orthoNear = x; }
	void setOrthoFar(float x) { orthoFar = x; }
	void setOrthographicVars(float orthoWidth, float orthoHeight, float orthoNear, float orthoFar) 
	{ setOrthoWidth(orthoWidth); setOrthoHeight(orthoHeight); setOrthoNear(orthoNear);  setOrthoFar(orthoFar); }

	void move(glm::vec3 value) { position += value; }
	void move(float x, float y, float z) { move(glm::vec3(x, y, z)); } 
	void move(float x, float z) { move(x, 0, z); }
	void moveForward(float speed) { position += lookAtVector * speed; }
	void moveBack(float speed) { position -= lookAtVector * speed; }
	void moveLeft(float speed) { position += leftVector * speed; }
	void moveRight(float speed) { position -= leftVector * speed; }
	void moveUp(float speed) { position += upVector * speed; }
	void moveDown(float speed) { position -= upVector * speed; }
	void setPosition(glm::vec3 value) { position = value; }
	void setPosition(float x, float y, float z) { setPosition(glm::vec3(x, y, z)); }
	void zeroVelocity() { velocity = glm::vec3(0,0,0); }
	void turn(long x, long y);
	void turn(POINT v) { turn(v.x, v.y); }
	void setUpVector(glm::vec3 x) { upVector = x; }

	glm::vec3 getPos()			{ return position; }
	glm::vec3 getAngle()		{ return glm::vec3(xAngle, yAngle, zAngle); }
	glm::vec3 getLookAtVector()	{ return lookAtVector; }
	glm::vec3 getUpVector()		{ return upVector; }
	glm::vec3 getLeftVector()	{ return leftVector; }

	void update(long dt);
private:
	inline void calculateLeftVector() { leftVector = glm::normalize(glm::cross(upVector, lookAtVector)); }
	static Camera * activeCamera;

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 lookAtVector;
	glm::vec3 upVector;
	glm::vec3 leftVector;

	float vFOV, aspectRatio, nearPlane, farPlane;
	float  orthoWidth, orthoHeight, orthoNear, orthoFar;
	float xAngle, yAngle, zAngle;

	glm::mat4 projection;
	glm::mat4 view;
};

#endif