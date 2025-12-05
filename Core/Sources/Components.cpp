#include "Core/Headers/Components.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

void Transform::setPosition(Vec3f position)
{
    m_position = position;
}

Vec3f Transform::getPosition() const
{
    return m_position;
}

void Transform::setScale(Vec3f scale)
{
    m_scale = scale;
}

void Transform::setScale(float scalar)
{
    m_scale = Vec3f(scalar);
}

Vec3f Transform::getScale() const
{
    return m_scale;
}

Vec3f Transform::getRotationAxis() const
{
    return m_currentRotationAxis;
}

float Transform::getRotationAngle() const
{
    return m_currentAngle;
}

void Transform::rotate(float angle, Vec3f rotationAxis)
{
    m_currentAngle = angle;
    m_currentRotationAxis = rotationAxis.getNormalized();
}

void DirectionalLight::setDirection(Vec3f dir)
{
    m_direction = dir;
}

Vec3f DirectionalLight::getDirection() const
{
   return m_direction;
}

// Mesh Renderer


