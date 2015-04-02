#include "Camera.h"

Camera * Camera::activeCamera = NULL;

Camera::Camera() 
{
	nearPlane = 0.1f;
	farPlane = 100.0f;
	vFOV = 50.0f;
	aspectRatio = (float)Settings::getWindowWidth()/(float)Settings::getWindowHeight();
	orthoHeight = 512;
	orthoWidth = 512;
	orthoNear = 0.0f;
	orthoFar = 100.0f;
	
	position = glm::vec3(0,7.5f,-30);
	velocity = glm::vec3(0,0,0);

	lookAtVector = glm::vec3(0,0,1);
	upVector = glm::vec3(0,1,0);
	leftVector = glm::vec3(1,0,0);
	xAngle = yAngle = zAngle = 0.0f;
}

Camera::~Camera()
{

}

int Camera::activate()
{
	if (activeCamera == this) 
		return 1;

	activeCamera = this;
	return 0;
}

void Camera::turn(long x, long y)
{
	float yVal = y * 0.1f;
	float xVal = x * 0.1f;

	if		(yAngle + yVal < -89.9f)	yVal = -89.9f - yAngle;
	else if (yAngle + yVal > 89.9f)		yVal = 89.9f - yAngle;
	yAngle += yVal;

	xAngle += xVal;
	if (xAngle > 180) xAngle -= 360.0f;
	if (xAngle < -180) xAngle += 360.0f;

	lookAtVector = glm::rotate(lookAtVector, yVal, leftVector);
	lookAtVector = glm::rotate(lookAtVector, xVal, -upVector);
	calculateLeftVector();
}

int Camera::calculateView()
{
	view = glm::lookAt(position, position+lookAtVector, upVector); 
	return 0;
}

int Camera::calculateProjection()
{
	projection = glm::perspective(vFOV, aspectRatio, nearPlane, farPlane);
	return 0;
}

int Camera::calculateOrthographicProjection()
{
	projection = glm::ortho(-orthoWidth/2.0f, orthoWidth/2.0f, -orthoHeight/2.0f, orthoHeight/2.0f, orthoNear, orthoFar);
	return 0;
}