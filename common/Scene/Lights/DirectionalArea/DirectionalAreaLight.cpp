#include "common/Scene/Lights/DirectionalArea/DirectionalAreaLight.h"

DirectionalAreaLight::DirectionalAreaLight(const glm::vec2& size):
    lightSize(size)
{
}

void DirectionalAreaLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;

    const glm::vec3 originObjPos = glm::vec3(GetWorldToObjectMatrix() * glm::vec4(origin, 1.f));

    if (   std::abs(originObjPos.x) < (0.5 * lightSize.x + SMALL_EPSILON)
        && std::abs(originObjPos.y) < (0.5 * lightSize.y + SMALL_EPSILON)
        && originObjPos.z < -SMALL_EPSILON
       )
    {
        const glm::vec3 lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(originObjPos.x, originObjPos.y, 0.f, 1.f));
        const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
        const float distanceToOrigin = glm::distance(origin, lightPosition);
        output.emplace_back(origin, rayDirection, distanceToOrigin);
    }
}

float DirectionalAreaLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f;
}

void DirectionalAreaLight::GenerateRandomPhotonRay(Ray& ray) const
{
    glm::vec3 rayPos;
    rayPos.x = (RandFloat01() - 0.5f) * lightSize.x;
    rayPos.y = (RandFloat01() - 0.5f) * lightSize.y;
    rayPos.z = 0.f;

    const glm::vec3 lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(rayPos, 1.f));

    ray.SetRayPosition(lightPosition);
    ray.SetRayDirection(glm::normalize(glm::vec3(GetForwardDirection())));
}
