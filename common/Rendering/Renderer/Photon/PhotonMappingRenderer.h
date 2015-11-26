#pragma once

#include "common/Rendering/Renderer.h"
#include "common/Rendering/Renderer/Photon/Photon.h"
#include <kdtree++/kdtree.hpp>
#include <functional>
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"

class PhotonMappingRenderer : public BackwardRenderer
{
public:
    PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler);
    virtual void InitializeRenderer() override;
    glm::vec3 ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const override;

    void SetNumberOfDiffusePhotons(int diffuse);
    void SetNumberOfSpecularPhotons(int specular);
    void SetPhotonSphereRadius(float radius);
    void SetPhotonGatherMultiplier(float multiplier);
private:
    using PhotonKdtree = KDTree::KDTree<3, Photon, PhotonAccessor>;
    PhotonKdtree diffuseMap;
    PhotonKdtree specularMap;

    int diffusePhotonNumber;
    int specularPhotonNumber;
    int maxPhotonBounces;
    
    float photonSphereRadius;
    float photonGatherMultiplier;

    std::pair<int, glm::vec3> GetPhotonIntensity(const class Light* currentLight, int totalPhotons);

    void GenericPhotonMapGeneration(int totalPhotons);
    void SpecularPhotonMapGeneration(int totalPhotons);

    void TracePhoton(bool specularPhotonPath, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces);
    void TraceSpecularPhoton(Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces);

};
