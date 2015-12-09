#include "assignment8/Assignment8.h"
#include "common/core.h"

//#define DO_DEPTH_OF_FIELD
#define DO_PHOTON_GATHERING
#define PHOTON_GATHERING_USES_SPHERE

std::shared_ptr<Camera> Assignment8::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<PerspectiveCamera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.0f);
#ifdef DO_DEPTH_OF_FIELD
    camera->SetZFocal(3.5);
    camera->SetApertureRadius(0.2);
#endif
    return camera;
}

std::shared_ptr<Scene> Assignment8::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
#ifdef DO_DEPTH_OF_FIELD
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment6-Test.obj", &loadedMaterials);
#elif defined(DO_PHOTON_GATHERING)

#ifdef PHOTON_GATHERING_USES_SPHERE
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Sphere.obj", &loadedMaterials);
#else
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment8.obj", &loadedMaterials);
#endif

#else
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment8.obj", &loadedMaterials);
#endif
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
#ifdef PHOTON_GATHERING_USES_SPHERE
        if (i == 0)
            materialCopy->SetReflectivity(0.6);
        else if (i == 1)
            materialCopy->SetTransmittance(0.9);
#endif
        cubeObjects[i]->SetMaterial(materialCopy);
    }

    std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
    cubeSceneObject->AddMeshObject(cubeObjects);
    cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
    newScene->AddSceneObject(cubeSceneObject);

    // Lights
#if !defined(DO_DEPTH_OF_FIELD) //&& !defined(DO_PHOTON_GATHERING)
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
#if defined(DO_PHOTON_GATHERING)

#ifdef PHOTON_GATHERING_USES_SPHERE
    pointLight->SetPosition(glm::vec3(0.f, 0.f, 1.57f));
#else
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
#endif

#else
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
#endif
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(pointLight);
#endif
    
#if defined(DO_DEPTH_OF_FIELD) //|| defined(DO_PHOTON_GATHERING)
#if defined(DO_PHOTON_GATHERING)

#ifdef PHOTON_GATHERING_USES_SPHERE
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(0.5f, 0.5f));
#if 1
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 4);
#else
    areaLight->SetSamplerAttributes(glm::vec3(8.f, 8.f, 1.f), 128);
#endif
    areaLight->SetPosition(glm::vec3(0.f, 0.f, 1.58f - LARGE_EPSILON));
#else
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(0.5f, 0.5f));
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 4);
    areaLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
#endif

#else
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(0.5f, 0.5f));
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 4);
    areaLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
#endif
    areaLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(areaLight);
#endif

    return newScene;

}
std::shared_ptr<ColorSampler> Assignment8::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(glm::ivec3(1, 1, 1));
    return jitter;
}

std::shared_ptr<class Renderer> Assignment8::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
#ifdef DO_PHOTON_GATHERING
    //return std::make_shared<BackwardRenderer>(scene, sampler);
    std::shared_ptr<class PhotonMappingRenderer>    photonRenderer = std::make_shared<PhotonMappingRenderer>(scene, sampler);
    //photonRenderer->SetNumberOfDiffusePhotons(2000000);
    photonRenderer->SetDiffusePhotonSphereRadius(0.03);
    photonRenderer->SetDiffusePhotonGatherMultiplier(4.0);
    photonRenderer->SetSpecularPhotonSphereRadius(0.03);
    photonRenderer->SetSpecularPhotonGatherMultiplier(4.0);
    return photonRenderer;
#else
    return std::make_shared<BackwardRenderer>(scene, sampler);
#endif
}

int Assignment8::GetSamplesPerPixel() const
{
    // ASSIGNMENT 5 TODO: Change the '1' here to increase the maximum number of samples used per pixel. (Part 1).
#ifdef DO_DEPTH_OF_FIELD
    return 16;
#elif defined(DO_PHOTON_GATHERING)
    return 1;
#else
    return 1;
#endif
}

bool Assignment8::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int Assignment8::GetMaxReflectionBounces() const
{
#if defined(DO_PHOTON_GATHERING) && defined(PHOTON_GATHERING_USES_SPHERE)
    return 2;
#else
    return 0;
#endif
}

int Assignment8::GetMaxRefractionBounces() const
{
#if defined(DO_PHOTON_GATHERING) && defined(PHOTON_GATHERING_USES_SPHERE)
    return 4;
#else
    return 0;
#endif
}

glm::vec2 Assignment8::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
