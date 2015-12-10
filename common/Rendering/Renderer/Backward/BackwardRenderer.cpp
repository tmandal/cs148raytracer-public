#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Rendering/Media/Media.h"
#include "common/Intersection/IntersectionState.h"

BackwardRenderer::BackwardRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) :
    Renderer(scene, sampler)
{
}

void BackwardRenderer::InitializeRenderer()
{
}

glm::vec3 BackwardRenderer::ComputeSampleColor(const IntersectionState& intersection, const Ray& fromCameraRay) const
{
    if (!intersection.hasIntersection) {
        return glm::vec3();
    }

    glm::vec3 intersectionPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
    assert(parentObject);

    const Material* objectMaterial = parentObject->GetMaterial();
    assert(objectMaterial);

    // Compute the color at the intersection.
    glm::vec3 sampleColor;
    for (size_t i = 0; i < storedScene->GetTotalLights(); ++i) {
        const Light* light = storedScene->GetLightObject(i);
        assert(light);

        // Sample light using rays, Number of samples and where to sample is determined by the light.
        std::vector<Ray> sampleRays;
        light->ComputeSampleRays(sampleRays, intersectionPoint, intersection.ComputeNormal());
        
        //std::cout << "Found " << sampleRays.size() << " from light source at " << glm::to_string(intersectionPoint) << std::endl;

        for (size_t s = 0; s < sampleRays.size(); ++s) {
            // note that max T should be set to be right before the light.
            IntersectionState shadowRayX(0, 0);  // To allow penetrating transimissive and participating media mesh
            glm::vec3   meshAttenuation = glm::vec3(1.f);
            storedScene->Trace(&sampleRays[s], &shadowRayX, &meshAttenuation, true);
            
            const float lightAttenuation = light->ComputeLightAttenuation(intersectionPoint);

            // Note that the material should compute the parts of the lighting equation too.
            const glm::vec3 brdfResponse = objectMaterial->ComputeBRDF(intersection, light->GetLightColor() * meshAttenuation, sampleRays[s], fromCameraRay, lightAttenuation);
            sampleColor += brdfResponse;
            //std::cout << "    BRDF with sample ray - meshAttenuation : " << glm::to_string(meshAttenuation) << " BRDF color : " << glm::to_string(brdfResponse) << std::endl;
        }
    }
    sampleColor += objectMaterial->ComputeNonLightDependentBRDF(this, intersection);
    return sampleColor;
}
