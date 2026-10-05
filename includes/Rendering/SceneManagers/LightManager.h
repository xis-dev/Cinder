#pragma once

#include <vector>

class DirectionalLight;
class LightEntity;

class LightManager {

private:
    std::vector<LightEntity*> m_lights;
    std::vector<DirectionalLight*> m_dirLights;
};

