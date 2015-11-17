#include "common/Rendering/Renderer/Photon/PhotonMappingRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/SceneObject.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "glm/gtx/component_wise.hpp"

#define VISUALIZE_PHOTON_MAPPING 1
//#define PHOTON_MAPPING_DEBUG

// Utility function
float RandFloat01()
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    maxPhotonBounces(1000)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber);
    diffuseMap.optimise();
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity = glm::length(currentLight->GetLightColor());
    }

    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }

        const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
        const int totalPhotonsForLight = static_cast<const int>(proportion * totalPhotons);
        const glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);
        for (int j = 0; j < totalPhotonsForLight; ++j) {
            Ray photonRay;
            std::vector<char> path;
            path.push_back('L');
            currentLight->GenerateRandomPhotonRay(photonRay);
            TracePhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
        }
    }
}

void PhotonMappingRenderer::TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
    /*
     * Assignment 7 TODO: Trace a photon into the scene and make it bounce.
     *    
     *    How to insert a 'Photon' struct into the photon map.
     *        Photon myPhoton;
     *        ... set photon properties ...
     *        photonMap.insert(myPhoton);
     */
    
    if (remainingBounces < 0)
        return;

    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;
    
    if (storedScene->Trace(photonRay, &state))
    {
        const glm::vec3     intersectionPoint = state.intersectionRay.GetRayPosition(state.intersectionT);
        const MeshObject*   hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
        const Material*     hitMaterial = hitMeshObject->GetMaterial();
        const glm::vec3     hitDiffuse = glm::normalize(hitMaterial->GetBaseDiffuseReflection());
        const float         reflectProb = (hitDiffuse.x > hitDiffuse.y)
                                ? (hitDiffuse.x > hitDiffuse.z ? hitDiffuse.x : hitDiffuse.z)
                                : (hitDiffuse.y > hitDiffuse.z ? hitDiffuse.y : hitDiffuse.z);
        float randProb = RandFloat01();

#ifdef PHOTON_MAPPING_DEBUG
        std::cout << "TracePhoton : hitDiffuse = " << glm::to_string(hitDiffuse) << " reflectProb = " << reflectProb << " randProb = " << randProb << std::endl;
#endif
        
        if (path.size() > 1)
        {
            Photon  photon;
            photon.position = intersectionPoint;
            photon.intensity = lightIntensity;
            photon.toLightRay = Ray(glm::vec3(photonRay->GetPosition()), -photonRay->GetRayDirection());
            photonMap.insert(photon);
        }
        
        if (randProb < reflectProb)
        {
            // Scatter
            
            path.push_back('R');
            
            // Hemisphere sampling
            float   u1 = RandFloat01();
            float   u2 = RandFloat01();
            
            float   r = sqrtf(u1);
            float   theta = 2 * PI * u2;
            
            float   x = r * cosf(theta);
            float   y = r * sinf(theta);
            float   z = sqrt(1 - u1);
            glm::vec3   drRayDirection = glm::normalize(glm::vec3(x, y, z));

#ifdef PHOTON_MAPPING_DEBUG
            std::cout << "Hemisphere sample ray direction = " << glm::to_string(drRayDirection) << std::endl;
#endif
            
            // Normal, Tangent and Bitangent vector generation
            glm::vec3   N = glm::normalize(state.ComputeNormal());
            glm::vec3   T;
            glm::vec3   B;
            
            if (N.x > LARGE_EPSILON || N.x < -LARGE_EPSILON || N.y > LARGE_EPSILON || N.y < -LARGE_EPSILON)
                T = glm::vec3(-N.y, N.x, 0.0);
            else
                T = glm::vec3(0.0, -N.z, N.y);
            B = glm::cross(N, T);
            
            T = glm::normalize(T);
            B = glm::normalize(B);

#ifdef PHOTON_MAPPING_DEBUG
            std::cout << "N = " << glm::to_string(N) << " T = " << glm::to_string(T) << " B = " << glm::to_string(B) << std::endl;
#endif
            
            glm::mat3   toWorldSpaceTransform = glm::mat3(T, B, N);
            
            // Construct diffuse reflection ray
            Ray diffuseReflectionRay;
            
            diffuseReflectionRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * N);
            diffuseReflectionRay.SetRayDirection(toWorldSpaceTransform * drRayDirection);
            
            TracePhoton(photonMap, &diffuseReflectionRay, lightIntensity, path, currentIOR, remainingBounces-1);
        }
        
    }
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundPhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundPhotons));
    if (!foundPhotons.empty()) {
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }
#endif
    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}
