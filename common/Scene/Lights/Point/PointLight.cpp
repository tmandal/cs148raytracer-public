#include "common/Scene/Lights/Point/PointLight.h"

float RandFloat()
{
    float r = -1.0f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX/2);
    
    return r;
}

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
        x = RandFloat();
        y = RandFloat();
        z = RandFloat();
    } while (x*x+y*y+z*z > 1.0f);
    
    ray.SetRayDirection(glm::normalize(glm::vec3(x, y, z)));
}