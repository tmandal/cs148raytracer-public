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

//#define VISUALIZE_PHOTON_MAPPING 1
//#define PHOTON_MAPPING_DEBUG
#define PHOTON_GATHERING_DEBUG

// Utility
float glm_max_component(glm::vec3 vector)
{
    return (vector.x > vector.y)
        ? (vector.x > vector.z ? vector.x : vector.z)
        : (vector.y > vector.z ? vector.y : vector.z);
}

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    maxPhotonBounces(1000),
    photonSphereRadius(0.003f)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffusePhotonNumber);
    diffuseMap.optimise();
    specularMap.optimise();
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity += glm::length(currentLight->GetLightColor());
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
            TracePhoton(true, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
        }
    }
}

void PhotonMappingRenderer::TracePhoton(bool specularPhotonPath, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
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
#if 0
        const glm::vec3     hitDiffuse = hitMaterial->GetBaseDiffuseReflection();
        const glm::vec3     hitSpecular = hitMaterial->IsReflective() ? hitMaterial->GetBaseSpecularReflection() : glm::vec3(0.0, 0.0, 0.0);
        const glm::vec3     hitReflection = hitDiffuse + hitSpecular;
        const float         probR = (hitReflection.x > hitReflection.y)
                                ? (hitReflection.x > hitReflection.z ? hitReflection.x : hitReflection.z)
                                : (hitReflection.y > hitReflection.z ? hitReflection.y : hitReflection.z);
        const float         probD = probR * (hitDiffuse.x + hitDiffuse.y + hitDiffuse.z) / (hitReflection.x + hitReflection.y + hitReflection.z);
#else
        const glm::vec3     hitDiffuse = hitMaterial->GetBaseDiffuseReflection();
        const glm::vec3     hitSpecular = hitMaterial->GetBaseSpecularReflection();
        const float         probD = glm_max_component(hitDiffuse * lightIntensity)  / glm_max_component(lightIntensity);
        const float         probS = glm_max_component(hitSpecular * lightIntensity) / glm_max_component(lightIntensity);
        const float         probR = probD + probS;
#endif
        
        float               randProb = RandFloat01();
        
        bool                isReflection = (randProb < probR);
        bool                isReflectionD = (randProb < probD);

#ifdef PHOTON_MAPPING_DEBUG
        //std::cout << "TracePhoton : hitDiffuse = " << glm::to_string(hitDiffuse) << " hitSpecular = " << glm::to_string(hitSpecular) << std::endl;
        //std::cout << "TracePhoton : hitReflection = " << glm::to_string(hitReflection) << " reflectProb = " << probR << " randProb = " << randProb << std::endl;
        //std::cout << "TracePhoton : reflectProb = " << probR << " randProb = " << randProb << std::endl;
#endif
        
        // Exclude photons from direct illumination
        if (path.size() > 1)
        {
            Photon  photon;
            photon.position = intersectionPoint;
            photon.intensity = lightIntensity;
            photon.toLightRay = Ray(glm::vec3(photonRay->GetPosition()), -photonRay->GetRayDirection());
#ifdef PHOTON_MAPPING_DEBUG
            std::cout << "Added photon : specularPhotonPath = " << specularPhotonPath << " position = " << glm::to_string(intersectionPoint) << " intensity = " << glm::to_string(lightIntensity) << std::endl;
#endif
            // If it's purely specular photon, add it to specular map
            if (specularPhotonPath)
            {
                specularMap.insert(photon);
            }
            // Always add the photon to diffuse map
            diffuseMap.insert(photon);
        }
        
        if (isReflection)
        {
            // Scatter
            
            glm::vec3   newLightIntensity = lightIntensity;
            
            if (isReflectionD)
            {
                path.push_back('D');
                newLightIntensity = lightIntensity * hitDiffuse / probD;
                specularPhotonPath = false; // Diffuse reflection => no longer specular photon path
            }
            else
            {
                path.push_back('S');
                newLightIntensity = lightIntensity * hitSpecular / probS;
            }
            
            // Hemisphere sampling
            float   u1 = RandFloat01();
            float   u2 = RandFloat01();
            
            float   r = sqrtf(u1);
            float   theta = 2 * PI * u2;
            
            float   x = r * cosf(theta);
            float   y = r * sinf(theta);
            float   z = sqrt(1 - u1);
            glm::vec3   sampleRay = glm::normalize(glm::vec3(x, y, z));

#ifdef PHOTON_MAPPING_DEBUG
            //std::cout << "Hemisphere sample ray direction = " << glm::to_string(sampleRay) << std::endl;
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
            //std::cout << "N = " << glm::to_string(N) << " T = " << glm::to_string(T) << " B = " << glm::to_string(B) << std::endl;
#endif
            
            glm::mat3   toWorldSpaceTransform = glm::mat3(T, B, N);
            
            // Construct  reflection ray
            Ray reflectionRay;
            
            reflectionRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * N);
            reflectionRay.SetRayDirection(toWorldSpaceTransform * sampleRay);
            
            TracePhoton(specularPhotonPath, &reflectionRay, newLightIntensity, path, currentIOR, remainingBounces-1);
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
    diffuseMap.find_within_range(intersectionVirtualPhoton, photonSphereRadius, std::back_inserter(foundPhotons));
    if (!foundPhotons.empty()) {
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }
#else
    if (intersection.hasIntersection) {
        const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
        assert(parentObject);
        
        const Material* objectMaterial = parentObject->GetMaterial();
        assert(objectMaterial);
        
        // Compute the color at the intersection using gathering photons.
        Photon intersectionVirtualPhoton;
        intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
        std::vector<Photon> foundDiffusePhotons;
        diffuseMap.find_within_range(intersectionVirtualPhoton, photonSphereRadius, std::back_inserter(foundDiffusePhotons));
        std::vector<Photon> foundSpecularPhotons;
        specularMap.find_within_range(intersectionVirtualPhoton, photonSphereRadius, std::back_inserter(foundSpecularPhotons));
        
        glm::vec3   diffuseMapGatherColor;
        for (size_t i = 0; i < foundDiffusePhotons.size(); ++i) {
            const Photon&   photon = foundDiffusePhotons[i];
            
            // Note that the material should compute the parts of the lighting equation too.
            //std::cout << "Photon : intensity = " << glm::to_string(photon.intensity) << ", ToLightRay = " << glm::to_string(photon.toLightRay.GetRayDirection()) <<  ", fromCameraRay=" << glm::to_string(fromCameraRay.GetRayDirection()) << std::endl;
            // Do diffuse BRDF
            const glm::vec3 brdfResponse = objectMaterial->ComputeBRDF(intersection, photon.intensity, photon.toLightRay, fromCameraRay, 1.0f, true, false);
            //std::cout << "BRDF response due to neighboring photons - " << glm::to_string(brdfResponse) << std::endl;
            diffuseMapGatherColor += brdfResponse;
        }
        diffuseMapGatherColor = diffuseMapGatherColor / (PI * photonSphereRadius * photonSphereRadius);
#ifdef PHOTON_GATHERING_DEBUG
        //if (foundDiffusePhotons.size() > 0)
        //    std::cout << "Color gathered from photon map of " << foundDiffusePhotons.size() << " neighboring diffuse photons - " << glm::to_string(diffuseMapGatherColor) << std::endl;
#endif
        finalRenderColor += diffuseMapGatherColor;
        
        glm::vec3   specularMapGatherColor;
        for (size_t i = 0; i < foundSpecularPhotons.size(); ++i) {
            const Photon&   photon = foundSpecularPhotons[i];
            
            // Note that the material should compute the parts of the lighting equation too.
            //std::cout << "Photon : intensity = " << glm::to_string(photon.intensity) << ", ToLightRay = " << glm::to_string(photon.toLightRay.GetRayDirection()) <<  ", fromCameraRay=" << glm::to_string(fromCameraRay.GetRayDirection()) << std::endl;
            // Do specular BRDF
            const glm::vec3 brdfResponse = objectMaterial->ComputeBRDF(intersection, photon.intensity, photon.toLightRay, fromCameraRay, 1.0f, true, false);
            //std::cout << "BRDF response due to neighboring photons - " << glm::to_string(brdfResponse) << std::endl;
            specularMapGatherColor += brdfResponse;
        }
        specularMapGatherColor = specularMapGatherColor / (PI * photonSphereRadius * photonSphereRadius);
#ifdef PHOTON_GATHERING_DEBUG
        if (foundSpecularPhotons.size() > 0)
            std::cout << "Color gathered from photon map of " << foundSpecularPhotons.size() << " neighboring specular photons - " << glm::to_string(specularMapGatherColor) << std::endl;
#endif
        finalRenderColor += specularMapGatherColor;
    }
    
    
#endif
    
    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}

void PhotonMappingRenderer::SetPhotonSphereRadius(float radius)
{
    photonSphereRadius = radius;
}
