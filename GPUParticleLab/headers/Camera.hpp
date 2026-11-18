#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	explicit Camera(float aspect);

	void Orbit(float deltaXPixels, float deltaYPixels);
	void Zoom(float scrollDelta);
	void SetAspect(float aspect);

	glm::vec3 GetPosition() const;
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewProjectionMatrix() const;

private:
	glm::vec3 m_target{ 0.0f };
	float m_distance = 4.0f;
	float m_yaw = 35.0f; //rotation around the Y-axis
	float m_pitch = 25.0f; //elevation angle from the XZ-plane
	float m_fov = 60.0f; //vertical field of view in degrees
	float m_aspect;
	float m_near = 0.1f;
	float m_far = 1000.0f;
	float m_rotateSpeed = 0.25f; //degrees per pixel
};