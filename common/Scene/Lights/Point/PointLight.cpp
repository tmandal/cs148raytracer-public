#include "common/Scene/Lights/Point/PointLight.h"
#include "common/utils.h"

void PointLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    output.emplace_back(origin, rayDirection, distanceToOrigin);
}

float PointLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f;
}

void PointLight::GenerateRandomPhotonRay(Ray& ray) const
{
    // Assignment 7 TODO: Fill in the random point light samples here.
    ray.SetRayPosition(glm::vec3(GetPosition()));
    
    float   x;
    float   y;
    float   z;
    
    do
    {
        x = -1.f + 2.f * RandFloat01();
        y = -1.f + 2.f * RandFloat01();
        z = -1.f + 2.f * RandFloat01();
    } while (x*x+y*y+z*z > 1.0f);
    
    ray.SetRayDirection(glm::normalize(glm::vec3(x, y, z)));
}
