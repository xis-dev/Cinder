#pragma once


#include "Component.h"

#include "vec2.hpp"
#include "vec3.hpp"

#include <variant>

#include "Delegate.h"

struct LightSettings {
    float m_intensity { 1.0f };
    glm::vec3 m_color { 1.0f };

};

struct DirectionalLightSettings: LightSettings
{
    glm::vec3 m_direction{0.0f, 0.0f, 1.0f};
};

struct PointLightSettings: LightSettings
{
    float m_attenuationRadius { 15.0f };
};

struct SpotLightSettings: LightSettings
{
    glm::vec3 m_direction {0.0f, 0.0f, 1.0f};
    float m_innerCutoff{};
    float m_outerCutoff{};
};

struct LightComponent: Component
{
    LightComponent() = default;
    enum class Type{Directional, Point, Spot} m_type{};
    LightComponent(Type);

    void changeType(LightComponent::Type type);
    Delegate<LightComponent::Type> OnTypeChanged;
    std::variant<DirectionalLightSettings, PointLightSettings, SpotLightSettings> m_settings{};

    struct Shadow {
        Shadow(LightComponent* owner);

        LightComponent* m_owner{nullptr};

        bool m_enabled{true};
        unsigned m_shadowMap{};
        glm::ivec2 m_resolution{};

        void changeResolution(int x, int y);
        Delegate<int, int> OnResolutionChanged;

        void changeShadowMapType(LightComponent::Type type);
        void recreateShadowMap(int x, int y);
    } m_shadow{this};

};

