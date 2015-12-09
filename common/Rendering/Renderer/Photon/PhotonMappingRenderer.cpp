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
//#define PHOTON_GATHERING_DEBUG
//#define FINAL_PHOTON_GATHERING

// Utility
float glm_max_component(glm::vec3 vector)
{
    return (vector.x > vector.y)
        ? (vector.x > vector.z ? vector.x : vector.z)
        : (vector.y > vector.z ? vector.y : vector.z);
}

glm::vec3 rand_point(const Box& box)
{
    float   xShift = RandFloat01();
    float   yShift = RandFloat01();
    float   zShift = RandFloat01();

    return (box.minVertex + glm::vec3(xShift, yShift, zShift) * (box.maxVertex - box.minVertex));
}

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    specularPhotonNumber(500000),
    maxPhotonBounces(1000),
    diffusePhotonSphereRadius(0.003f),
    diffusePhotonGatherMultiplier(1.0f),
    specularPhotonSphereRadius(0.003f),
    specularPhotonGatherMultiplier(1.0f)
{
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffusePhotonNumber);
    SpecularPhotonMapGeneration(specularPhotonNumber);
    diffuseMap.optimise();
    specularMap.optimise();
}

std::pair<int, glm::vec3> PhotonMappingRenderer::GetPhotonIntensity(const class Light* currentLight, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    for (size_t i = 0; i < storedScene->GetTotalLights(); ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity += glm::length(currentLight->GetLightColor());
    }

    const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
    const int totalPhotonsForLight = static_cast<const int>(proportion * (diffusePhotonNumber + specularPhotonNumber));
    int totalPhotonsToEmit = static_cast<const int>(proportion * totalPhotons);
    glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);

    return  std::make_pair(totalPhotonsToEmit, photonIntensity);
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(int totalPhotons)
{
    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    for (size_t i = 0; i < storedScene->GetTotalLights(); ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }

        std::pair<int, glm::vec3>   pkg = GetPhotonIntensity(currentLight, totalPhotons);
        const int totalPhotonsForLight = pkg.first;
        const glm::vec3 photonIntensity = pkg.second;
        for (int j = 0; j < totalPhotonsForLight; ++j) {
            Ray photonRay;
            std::vector<char> path;
            path.push_back('L');
            currentLight->GenerateRandomPhotonRay(photonRay);
            //std::cout << "TracePhoton " << j << std::endl;
            TracePhoton(true, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
        }
    }
}

void PhotonMappingRenderer::SpecularPhotonMapGeneration(int totalPhotons)
{
    // Get a list of specular objects 
    std::vector<std::pair<const MeshObject*, const SceneObject*>>      specularMeshObjects;
    for (size_t i = 0; i < storedScene->GetTotalObjects(); ++i) {
        const SceneObject&  sceneObject = storedScene->GetSceneObject(i);
        for (int j = 0; j < sceneObject.GetTotalMeshObjects(); ++j) {
            const MeshObject*   meshObject = sceneObject.GetMeshObject(j);
            assert(meshObject);
            const Material*     objectMaterial = meshObject->GetMaterial();
            assert(objectMaterial);
            if (objectMaterial->IsReflective() || objectMaterial->IsTransmissive()) {
                specularMeshObjects.push_back(std::make_pair(meshObject, &sceneObject));
            }
        }
    }

    // Early return if no specular objects are found
    if (specularMeshObjects.size() == 0)
        return;

    for (size_t i = 0; i < storedScene->GetTotalLights(); ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }

        std::pair<int, glm::vec3>   pkg = GetPhotonIntensity(currentLight, totalPhotons);
        const int totalPhotonsForLight = pkg.first;
        const glm::vec3 photonIntensity = pkg.second;
        for (int j = 0; j < totalPhotonsForLight; ++j) {
            Ray photonRay;
            std::vector<char> path;
            path.push_back('L');
            currentLight->GenerateRandomPhotonRay(photonRay);
            // Modify photon ray direction to point towards a specular object
            size_t  objIndex = rand() % specularMeshObjects.size();
            Box     objBBox = specularMeshObjects[objIndex].first->GetBoundingBox();
            glm::vec4   randObjPoint = specularMeshObjects[objIndex].second->GetObjectToWorldMatrix() * glm::vec4(rand_point(objBBox), 1.f);
            photonRay.SetRayDirection(glm::normalize(glm::vec3(randObjPoint - photonRay.GetPosition())));
            // TODO : Redo initial photon ray construction if it does not hit a specular object

            //std::cout << "TraceSpecularPhoton " << j << std::endl;
            TraceSpecularPhoton(&photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
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
            if (specularPhotonPath && isReflectionD)
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

void PhotonMappingRenderer::TraceSpecularPhoton(Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
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

        const float         probSR = hitMaterial->GetReflectivity();
        const float         probST = hitMaterial->GetTransmittance();
        const float         probS = probSR + probST;
        
        float               randProb = RandFloat01();
        
        bool                isSpecular  = (randProb < probS);
        bool                isSpecularR = (randProb < probSR);

#ifdef PHOTON_MAPPING_DEBUG
        //std::cout << "TracePhoton : reflectProb = " << probR << " randProb = " << randProb << std::endl;
#endif
        
        // Exclude photons from direct illumination
        if (path.size() > 1 && probS < SMALL_EPSILON)
        {
            Photon  photon;
            photon.position = intersectionPoint;
            photon.intensity = lightIntensity;
            photon.toLightRay = Ray(glm::vec3(photonRay->GetPosition()), -photonRay->GetRayDirection());
#ifdef PHOTON_MAPPING_DEBUG
            std::cout << "Added specular photon : position = " << glm::to_string(intersectionPoint) << " intensity = " << glm::to_string(lightIntensity) << std::endl;
#endif
            // If it's purely specular photon, add it to specular map
            specularMap.insert(photon);
        }
        
        if (isSpecular)
        {
            // Scatter
            
            glm::vec3   newLightIntensity = lightIntensity;
            Ray         outputPhotonRay;
            float       targetIOR = currentIOR;
            const float NdR = glm::dot(photonRay->GetRayDirection(), state.ComputeNormal());
            
            if (isSpecularR)
            {
                // Specular reflection
                path.push_back('R');
                newLightIntensity = lightIntensity * probSR;
                // Reflect incoming photon ray
                const glm::vec3 normal = (NdR > SMALL_EPSILON) ? -1.f * state.ComputeNormal() : state.ComputeNormal();
                const glm::vec3 reflectionDir = glm::reflect(photonRay->GetRayDirection(), normal);
                outputPhotonRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * state.ComputeNormal());
                outputPhotonRay.SetRayDirection(reflectionDir);
            }
            else
            {
                // Specular transmission
                path.push_back('T');
                newLightIntensity = lightIntensity * probST;
                // Refract incoming photon ray
                targetIOR = (NdR < SMALL_EPSILON) ? hitMaterial->GetIOR() : 1.f;
                const glm::vec3 refractionDir = photonRay->RefractRay(state.ComputeNormal(), currentIOR, targetIOR);
                outputPhotonRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * refractionDir);
                outputPhotonRay.SetRayDirection(refractionDir);
            }
            
            TraceSpecularPhoton(&outputPhotonRay, newLightIntensity, path, targetIOR, remainingBounces-1);
        }
    }
}


glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor;
    finalRenderColor += BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundDiffusePhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundDiffusePhotons));
    if (!foundDiffusePhotons.empty()) {
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }

    std::vector<Photon> foundSpecularPhotons;
    specularMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundSpecularPhotons));
    if (!foundSpecularPhotons.empty()) {
        finalRenderColor += glm::vec3(0.f, 1.f, 0.f);
    }
#elif defined(FINAL_PHOTON_GATHERING)
    if (intersection.hasIntersection) {
        int finalGatherRayNumber = 10;
        const glm::vec3     intersectionPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
        glm::vec3   finalGatherColor;
        int         totalFinalGatherRays = 0;
        for (int i = 0; i < finalGatherRayNumber; ++i)
        {
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
            glm::vec3   N = glm::normalize(intersection.ComputeNormal());
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
            
            // Construct  final gather ray
            Ray finalGatherRay;
            
            finalGatherRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * N);
            finalGatherRay.SetRayDirection(toWorldSpaceTransform * sampleRay);
            
            IntersectionState state(0, 0);
            if (storedScene->Trace(&finalGatherRay, &state))
            {
                finalGatherColor += ComputePhotonMapColor(intersection, fromCameraRay);
                ++totalFinalGatherRays;
            }
        }
        
        if (totalFinalGatherRays> 0)
            finalRenderColor += finalGatherColor / (float)totalFinalGatherRays;
    }
#else
    if (intersection.hasIntersection) {
        finalRenderColor += ComputePhotonMapColor(intersection, fromCameraRay);
    }
#endif
    
    return finalRenderColor;
}

glm::vec3 PhotonMappingRenderer::ComputePhotonMapColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3   photonMapRenderColor;
    
    const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
    assert(parentObject);
    
    const Material* objectMaterial = parentObject->GetMaterial();
    assert(objectMaterial);
    
    // Compute the color at the intersection using gathering photons.
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    std::vector<Photon> foundDiffusePhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, diffusePhotonSphereRadius, std::back_inserter(foundDiffusePhotons));
    std::vector<Photon> foundSpecularPhotons;
    specularMap.find_within_range(intersectionVirtualPhoton, specularPhotonSphereRadius, std::back_inserter(foundSpecularPhotons));
    
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
    diffuseMapGatherColor = diffuseMapGatherColor / (PI * diffusePhotonSphereRadius * diffusePhotonSphereRadius);
#ifdef PHOTON_GATHERING_DEBUG
    //if (foundDiffusePhotons.size() > 0)
    //    std::cout << "Color gathered from photon map of " << foundDiffusePhotons.size() << " neighboring diffuse photons - " << glm::to_string(diffuseMapGatherColor) << std::endl;
#endif
    photonMapRenderColor += diffuseMapGatherColor * diffusePhotonGatherMultiplier;
    
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
    specularMapGatherColor = specularMapGatherColor / (PI * specularPhotonSphereRadius * specularPhotonSphereRadius);
#ifdef PHOTON_GATHERING_DEBUG
    if (foundSpecularPhotons.size() > 0)
        std::cout << "Color gathered from photon map of " << foundSpecularPhotons.size() << " neighboring specular photons - " << glm::to_string(specularMapGatherColor) << std::endl;
#endif
    photonMapRenderColor += specularMapGatherColor * specularPhotonGatherMultiplier;
    
    return photonMapRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}

void PhotonMappingRenderer::SetNumberOfSpecularPhotons(int specular)
{
    specularPhotonNumber = specular;
}

void PhotonMappingRenderer::SetDiffusePhotonSphereRadius(float radius)
{
    diffusePhotonSphereRadius = radius;
}

void PhotonMappingRenderer::SetDiffusePhotonGatherMultiplier(float multiplier)
{
    diffusePhotonGatherMultiplier = multiplier;
}

void PhotonMappingRenderer::SetSpecularPhotonSphereRadius(float radius)
{
    specularPhotonSphereRadius = radius;
}

void PhotonMappingRenderer::SetSpecularPhotonGatherMultiplier(float multiplier)
{
    specularPhotonGatherMultiplier = multiplier;
}
