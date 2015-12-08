#pragma once

#include "common/Scene/Lights/Light.h"

class DirectionalAreaLight : public Light
{
public:
    DirectionalAreaLight(const glm::vec2& size, const glm::vec3& direction);

    virtual void ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const override;
    virtual float ComputeLightAttenuation(glm::vec3 origin) const override;

    virtual void GenerateRandomPhotonRay(Ray& ray) const override;

private:
    glm::vec2 lightSize;
    glm::vec3 lightDirection;
};
