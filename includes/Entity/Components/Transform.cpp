#include "Transform.h"

#include "Entity.h"

glm::vec3 Transform::getWorldPosition(const Entity &e)
{
    glm::vec3 outPos{};

    const auto* currentEnt = &e;
    while (currentEnt)
    {
        if (const auto* t = currentEnt->getComponent<Transform>())
        {
            outPos += t->m_position;
            currentEnt = currentEnt->getParent();
            continue;
        }
        break;
    }

    return outPos;
}

glm::mat4 Transform::getGlobalTransform(const Entity &e)
{
    const Entity* currentEnt = &e;

    std::vector<const Transform*> transformChain;

    while (currentEnt)
    {
        if (const auto* t = currentEnt->getComponent<Transform>())
        {
            transformChain.push_back(t);
            currentEnt = currentEnt->getParent();
            continue;
        }

        break;

    }

    glm::mat4 result {1.0f};
    // Apply root transform first and go down chain
    for (const auto& t : std::views::reverse(transformChain))
    {
        glm::mat4 local = getTransformMatrix(*t);
        result = result * local;
    }

    return result;
}

glm::mat4 Transform::getTransformMatrix(const Transform &t)
{
    glm::mat4 out {1.0f};
    out = glm::translate(out, t.m_position);
    out = glm::rotate(out,  glm::radians(t.m_rotationAngle), t.m_rotationAxis);
    out = glm::scale(out, t.m_scale);

    return out;
}
