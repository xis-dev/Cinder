#include "LightComponent.h"

#include "Texture.h"



LightComponent::LightComponent(Type type)
{
    changeType(type);
}

void LightComponent::changeType(LightComponent::Type type)
{
    m_type = type;
    switch (m_type)
    {
        case Type::Directional:
            m_settings = DirectionalLightSettings{};
            break;

        case Type::Point:
            m_settings = PointLightSettings{};
            break;

        case Type::Spot:
            m_settings = SpotLightSettings{};
            break;
    }
    OnTypeChanged.broadcast(type);
}

LightComponent::Shadow::Shadow(LightComponent *owner): m_owner(owner)
{
    recreateShadowMap(m_resolution.x, m_resolution.y);

    m_owner->OnTypeChanged.bindFunction(this, &Shadow::changeShadowMapType);
   OnResolutionChanged.bindFunction(this, &Shadow::recreateShadowMap);
}

void LightComponent::Shadow::changeResolution(int x, int y)
{
    m_resolution.x = x;
    m_resolution.y = y;
    OnResolutionChanged.broadcast(x, y);
}

void LightComponent::Shadow::changeShadowMapType(LightComponent::Type)
{
    recreateShadowMap(m_resolution.x, m_resolution.y);
}

void LightComponent::Shadow::recreateShadowMap(int x, int y)
{
    if (!m_owner) return;
    // TODO: Abstraction for shadow maps to not use glObjects
    if (m_shadowMap != 0) glDeleteTextures(1, &m_shadowMap);

    switch (m_owner->m_type)
    {
        case Type::Directional:
            m_shadowMap = Texture::createEmptyTex(x, y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
            break;

        case Type::Point:
            m_shadowMap = Texture::createEmptyCubemap(x, y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
            break;

        case Type::Spot:
            break;
        default:
            break;
    }
}

