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
    void SetPhotonSphereRadius(float radius);
private:
    using PhotonKdtree = KDTree::KDTree<3, Photon, PhotonAccessor>;
    PhotonKdtree diffuseMap;
    PhotonKdtree specularMap;

    int diffusePhotonNumber;
    int maxPhotonBounces;
    
    float photonSphereRadius;

    void GenericPhotonMapGeneration(int totalPhotons);
    void TracePhoton(bool specularPhotonPath, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces);
};