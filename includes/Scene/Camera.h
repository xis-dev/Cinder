#pragma once

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <cmath>


class Camera
{
public:
	Camera(glm::vec3 position,glm::vec3 direction, float maxFov, float aspect, float movementSpeed, float nearPlane = 0.01f, float farPlane = 10000.0f):
	m_position(position),  m_direction(glm::normalize(direction)),m_maxFov(maxFov), m_currentFov(maxFov),
	m_aspectRatio(aspect), m_nearPlane(nearPlane), m_farPlane(farPlane), m_speed(movementSpeed)
	{
		m_yaw = glm::degrees(std::atan2f(m_direction.z, m_direction.x));
		m_pitch = glm::degrees(std::asinf(m_direction.y));
		setCameraDirection(m_yaw, m_pitch);
		setViewMatrix();
		setProjectionMatrix();
	}


private:
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_position{};
	glm::vec3 m_direction{};
	glm::vec3 m_right{};
	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};

	float m_maxFov{};
	float m_currentFov{};
	float m_speed{};
	float m_yaw{};
	float m_pitch{};

	float m_aspectRatio{};
	float m_nearPlane{};
	float m_farPlane{};

	void setViewMatrix();
	void setProjectionMatrix();
	void setCameraDirection(float yaw, float pitch);

public:
	void processKeyboardInput(GLFWwindow* window, float deltaTime);
	void processMouseInput(GLFWwindow* window, float xoffset, float yoffset, float zoomAmount, float sensitivity = 0.1f);
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;
	glm::vec3 getRightVector() const;
	glm::vec3 getUpVector() const;

};
