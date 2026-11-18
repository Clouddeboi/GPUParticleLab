#include "../headers/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

Camera::Camera(float aspect) : m_aspect(aspect) {}

//Orbit the camera around the target point based on mouse movement in pixels.
void Camera::Orbit(float deltaXPixels, float deltaYPixels)
{
	m_yaw -= deltaXPixels * m_rotateSpeed;
	m_pitch = std::clamp(m_pitch + deltaYPixels * m_rotateSpeed, -89.0f, 89.0f);
}

void Camera::Zoom(float scrollDelta)
{
	m_distance = std::clamp(m_distance * std::exp(-scrollDelta * -.1f), 0.5f, 200.0f);
}

void Camera::SetAspect(float aspect)
{
	m_aspect = aspect;
}

//Calculates the camera's position in world space 
//based on its yaw, pitch, and distance from the target point.
glm::vec3 Camera::GetPosition() const
{
	//Convert degrees to radians for trigonometric functions
	const float yaw = glm::radians(m_yaw);
	const float pitch = glm::radians(m_pitch);

	//Calculate the offset from the target point using spherical coordinates
	const glm::vec3 offset(
		std::cos(pitch) * std::sin(yaw), //Horizontal Position
		std::sin(pitch), //Vertical Position
		std::cos(pitch) * std::cos(yaw) //Depth Position
	);

	return m_target + m_distance * offset;
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(GetPosition(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetViewMatrix();
}