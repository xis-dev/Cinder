#include "Objects/Headers/Camera.h"

const glm::vec3 Camera::m_up = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 Camera::getProjectionMatrix()
{
	return m_projectionMatrix;
}

glm::mat4 Camera::getViewMatrix()
{
	return m_viewMatrix;
}

void Camera::setCameraDirection(float yaw, float pitch)
{
	m_direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	m_direction.y = std::sin(glm::radians(pitch));
	m_direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

	if (std::abs(m_direction.x) < 1e-6f) m_direction.x = 0.0f;
	if (std::abs(m_direction.y) < 1e-6f) m_direction.y = 0.0f;
	if (std::abs(m_direction.z) < 1e-6f) m_direction.z = 0.0f;


	m_direction = glm::normalize(m_direction);
}

void Camera::processKeyboardInput(GLFWwindow* window, float deltaTime)
{
	float speed = m_speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed *= 2.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_W)  == GLFW_PRESS || 
		glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		m_position += m_direction * (speed * deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		m_position -= m_direction * (speed * deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		m_position -= glm::normalize(glm::cross(m_up, m_direction)) * speed * deltaTime;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		m_position += glm::normalize(glm::cross(m_up, m_direction)) * speed * deltaTime;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		m_position += (m_up * speed * deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		m_position -= m_up * speed * deltaTime;
	}

	setViewMatrix();
}

void Camera::processMouseInput(GLFWwindow* window, float xoffset, float yoffset, float zoomAmount, float sensitivity)
{
	m_yaw += (xoffset * sensitivity);
	m_pitch += (yoffset * sensitivity);

	m_pitch = std::min(m_pitch, 89.0f);
	m_pitch = std::max(m_pitch, -89.0f);

	setCameraDirection(m_yaw, m_pitch);
	

	m_currentFov -= zoomAmount;

	m_currentFov = std::max(m_currentFov, 1.0f);
	m_currentFov = std::min(m_currentFov, m_maxFov);

	setViewMatrix();
	setProjectionMatrix();


}


void Camera::setViewMatrix()
{
	m_viewMatrix = glm::lookAt(m_position, m_position + m_direction, m_up);
}

void Camera::setProjectionMatrix()
{
	m_projectionMatrix = glm::perspective(glm::radians(m_currentFov), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::vec3 Camera::getPosition() const
{
	return m_position;
}

glm::vec3 Camera::getDirection() const
{
	return m_direction;
}


