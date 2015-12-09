//
//  VolumeLight.cpp
//  cs148-raytracer
//
//  Created by Tanmoy on 11/30/15.
//
//

#include "VolumeLight.h"
#include "common/Scene/Geometry/Primitives/PrimitiveBase.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"

void VolumeLight::ComputeTriangleAreas()
{
    if (primitiveAreas.size() > 0)
        return;
    assert(GetTotalMeshObjects() > 0);
    primitiveAreas.clear();
    totalPrimitiveAreas = 0.f;
    for (int m = 0; m < GetTotalMeshObjects(); ++m)
    {
        const MeshObject*   meshObject = GetMeshObject(m);
        for (int p = 0; p < meshObject->GetTotalPrimitives(); ++p)
        {
            const PrimitiveBase*    primitive = meshObject->GetPrimitive(p);
            assert(primitive->GetTotalVertices() == 3);
            primitiveAreas.push_back(fabs(0.5 * glm::length(primitive->GetPrimitiveNormal())));
            totalPrimitiveAreas += *primitiveAreas.rbegin();
        }
    }
}

void VolumeLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    assert(GetTotalMeshObjects() > 0);
    origin += normal * LARGE_EPSILON;
    for (int i = 0; i < samplesToUse; ++i)
    {
        size_t                  m = rand()%GetTotalMeshObjects();
        const MeshObject*       meshObject = GetMeshObject(m);
        size_t                  p = rand()%meshObject->GetTotalPrimitives();
        const PrimitiveBase*    primitive = meshObject->GetPrimitive(p);
        glm::vec3               sample = primitive->GetVertexPosition(0) + RandFloat01() * (primitive->GetVertexPosition(1) - primitive->GetVertexPosition(0)) + RandFloat01() * (primitive->GetVertexPosition(2) - primitive->GetVertexPosition(1));
        glm::vec3               lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(sample, 1.f));
        const glm::vec3         rayDirection = glm::normalize(lightPosition - origin);
        lightPosition -= rayDirection * LARGE_EPSILON;
        const float             distanceToOrigin = glm::distance(origin, lightPosition);
        const glm::vec3         lightNormal = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(primitive->GetPrimitiveNormal(), 1.f));
        if (glm::dot(rayDirection, lightNormal) < - SMALL_EPSILON)
            output.emplace_back(origin, rayDirection, distanceToOrigin);
        else    // retry
            --i;
    }
}

float VolumeLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.0 / samplesToUse /* * (areaCovered/totalPrimitiveAreas)*/;
}

void VolumeLight::GenerateRandomPhotonRay(Ray& ray) const
{
    assert(GetTotalMeshObjects() > 0);
    
    size_t                  m = rand()%GetTotalMeshObjects();
    const MeshObject*       meshObject = GetMeshObject(m);
    size_t                  p = rand()%meshObject->GetTotalPrimitives();
    const PrimitiveBase*    primitive = meshObject->GetPrimitive(p);
    glm::vec3               sample = primitive->GetVertexPosition(0) + RandFloat01() * (primitive->GetVertexPosition(1) - primitive->GetVertexPosition(0)) + RandFloat01() * (primitive->GetVertexPosition(2) - primitive->GetVertexPosition(1));
    const glm::vec3         lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(sample, 1.f));
    
    ray.SetRayPosition(lightPosition);
    
    // Hemisphere sampling
    float   u1 = RandFloat01();
    float   u2 = RandFloat01();
    
    float   r = sqrtf(u1);
    float   theta = PI * u2;
    
    float   x = r * cosf(theta);
    float   y = r * sinf(theta);
    float   z = sqrt(1 - u1);
    
    // Normal, Tangent and Bitangent vector generation
    glm::vec3   N = glm::normalize(primitive->GetPrimitiveNormal());
    glm::vec3   T;
    glm::vec3   B;
    
    if (N.x > LARGE_EPSILON || N.x < -LARGE_EPSILON || N.y > LARGE_EPSILON || N.y < -LARGE_EPSILON)
        T = glm::vec3(-N.y, N.x, 0.0);
    else
        T = glm::vec3(0.0, -N.z, N.y);
    B = glm::cross(N, T);
    
    T = glm::normalize(T);
    B = glm::normalize(B);
    
    glm::mat3   toObjectSpaceTransform = glm::mat3(T, B, N);
    
    const glm::vec3 lightDirection = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(toObjectSpaceTransform * glm::vec3(x, y, z), 1.f));
    
    ray.SetRayDirection(glm::normalize(lightDirection));
}

// Sampler Attributes
void VolumeLight::SetNumSamples(int numSamples)
{
    samplesToUse = numSamples;
}
