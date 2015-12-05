#include "common/Scene/Scene.h"
#include "common/Scene/Geometry/Ray/Ray.h"
#include "common/Scene/Geometry/Primitives/PrimitiveBase.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Acceleration/AccelerationCommon.h"

void Scene::GenerateDefaultAccelerationData()
{
    if (!acceleration) {
        GenerateAccelerationData(AccelerationTypes::NONE);
    }

    assert(acceleration);

    for (size_t i = 0; i < sceneObjects.size(); ++i) {
        sceneObjects[i]->CreateDefaultAccelerationData();
    }
}

AccelerationStructure* Scene::GenerateAccelerationData(AccelerationTypes inputType)
{
    acceleration = AccelerationGenerator::CreateStructureFromType(inputType);
    return acceleration.get();
}


HitStatus Scene::Trace(class Ray* inputRay, IntersectionState* outputIntersection, bool isShadowRay) const
{
    assert(inputRay);
    DIAGNOSTICS_STAT(DiagnosticsType::RAYS_CREATED);

    bool didIntersect = acceleration->Trace(nullptr, inputRay, outputIntersection);
    HitStatus   indirectHitStatus = HitStatus::HIT_NONE;
    if (outputIntersection != nullptr && didIntersect) {
        const MeshObject* intersectedMesh = outputIntersection->intersectedPrimitive->GetParentMeshObject();
        assert(intersectedMesh);
        const Material* currentMaterial = intersectedMesh->GetMaterial();
        assert(currentMaterial);

        // Trace through the object if it's not a shadow ray of it's participating media
        if (!isShadowRay || intersectedMesh->IsMedia())    {

            const glm::vec3 intersectionPoint = outputIntersection->intersectionRay.GetRayPosition(outputIntersection->intersectionT);
            const float NdR = glm::dot(inputRay->GetRayDirection(), outputIntersection->ComputeNormal());
            // send out reflection ray.
            if (currentMaterial->IsReflective() && outputIntersection->remainingReflectionBounces > 0) {
                outputIntersection->reflectionIntersection = std::make_shared<IntersectionState>(outputIntersection->remainingReflectionBounces - 1, outputIntersection->remainingRefractionBounces);
         
                Ray reflectionRay;
                PerformRaySpecularReflection(reflectionRay, *inputRay, intersectionPoint, NdR, *outputIntersection);
                indirectHitStatus = Trace(&reflectionRay, outputIntersection->reflectionIntersection.get());
            }
         
            // send out refraction ray.
            if (currentMaterial->IsTransmissive() && outputIntersection->remainingRefractionBounces > 0) {
                outputIntersection->refractionIntersection = std::make_shared<IntersectionState>(outputIntersection->remainingReflectionBounces, outputIntersection->remainingRefractionBounces - 1);
         
                // If we're going into the mesh, set the target IOR to be the IOR of the mesh.
                float targetIOR = (NdR < SMALL_EPSILON) ? currentMaterial->GetIOR() : 1.f;
         
                Ray refractionRay;
                PerformRayRefraction(refractionRay, *inputRay, intersectionPoint, NdR, *outputIntersection, targetIOR);
                outputIntersection->refractionIntersection->currentIOR = targetIOR;
                //std::cout << "Refraction trace : remainingBounces=" <<  outputIntersection->remainingRefractionBounces - 1 << " inputRay - position=" << glm::to_string(inputRay->GetPosition()) << ", direction=" << glm::to_string(inputRay->GetRayDirection()) << "refractionRay - position=" << glm::to_string(refractionRay.GetPosition()) << ", direction=" << glm::to_string(refractionRay.GetRayDirection()) << std::endl;
                indirectHitStatus = Trace(&refractionRay, outputIntersection->refractionIntersection.get());
            }
        }
    }

    if (didIntersect)
        if (outputIntersection != nullptr && outputIntersection->intersectedPrimitive->GetParentMeshObject()->IsMedia() && indirectHitStatus != HitStatus::HIT_OBJECTS)
            return HitStatus::HIT_MEDIA_ONLY;
        else
            return HitStatus::HIT_OBJECTS;
    else
        return HitStatus::HIT_NONE;
}

void Scene::PerformRaySpecularReflection(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state) const
{
    const glm::vec3 normal = (NdR > SMALL_EPSILON) ? -1.f * state.ComputeNormal() : state.ComputeNormal();
    const glm::vec3 reflectionDir = glm::reflect(inputRay.GetRayDirection(), normal);
    outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * reflectionDir);
    outputRay.SetRayDirection(reflectionDir);
}

void Scene::PerformRayRefraction(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state, float& targetIOR) const
{
    const glm::vec3 refractionDir = inputRay.RefractRay(state.ComputeNormal(), state.currentIOR, targetIOR);
    outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * refractionDir);
    outputRay.SetRayDirection(refractionDir);
}

void Scene::AddSceneObject(std::shared_ptr<SceneObject> object)
{
    if (!object) {
        return;
    }
    sceneObjects.emplace_back(std::move(object));
}

void Scene::AddLight(std::shared_ptr<Light> light)
{
    if (!light) {
        return;
    }
    sceneLights.emplace_back(std::move(light));
}

void Scene::Finalize()
{
    for (size_t i = 0; i < sceneObjects.size(); ++i) {
        sceneObjects[i]->Finalize();
    }
    assert(acceleration);
    acceleration->Initialize(sceneObjects);
}
