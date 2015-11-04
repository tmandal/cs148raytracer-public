#include "Scanline.h"
#include "common/core.h" // <-- haha.
#include "common/Utility/Mesh/Loading/MeshLoader.h"
#include <cmath>
#include "LandscapeModel.hpp"

#define MOVING_SCALE	100.0f

std::shared_ptr<Camera> Scanline::CreateCamera() const
{
#if 1
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.0f);
#else
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(5100.f, 500.0f, 100.f));
    camera->Rotate(glm::vec3(SceneObject::GetWorldRight()), -PI/6.0);
    
    camera->Rotate(glm::vec3(camera->GetRightDirection()), 0.3f);
    camera->Translate(glm::vec3(camera->GetForwardDirection() * -MOVING_SCALE));
    camera->Translate(glm::vec3(camera->GetForwardDirection() * -MOVING_SCALE));
    camera->Translate(glm::vec3(camera->GetForwardDirection() * -MOVING_SCALE));
    camera->Translate(glm::vec3(camera->GetForwardDirection() * -MOVING_SCALE));
    camera->Translate(glm::vec3(SceneObject::GetWorldUp() * MOVING_SCALE));
#endif
    
    return camera;
}

std::shared_ptr<Scene> Scanline::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
    
    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    
#if 0
    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Original.obj", &loadedMaterials);
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        cubeObjects[i]->SetMaterial(materialCopy);
    }
    
    std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
    cubeSceneObject->AddMeshObject(cubeObjects);
    cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    cubeSceneObject->CreateAccelerationData(AccelerationTypes::NONE);
    newScene->AddSceneObject(cubeSceneObject);
    
#else
    
    // My object models
    
#if 1
    // Landscape models
#if 1
    float		terrainZLength = 2.0;
    float		terrainXWidth = 2.0;
    float		terrainMaxYHeight = 0.2;
    float		roadWidth = 0.5f;
    float		roadPivotWidthX = 1.0;
    unsigned	segmentSteps = 10;
    unsigned    roadStepsScale = 10;
    float       mtnSegmentLength = 0.25;
#else
    float		terrainZLength = 2000.0;
    float		terrainXWidth = 10000.0;
    float		terrainMaxYHeight = 200.0;
    float		roadWidth = 800.0f;
    float		roadPivotWidthX = 1000.0;
    unsigned	segmentSteps = 100;
    float       mtnSegmentLength = 100;
    unsigned    roadStepsScale = 100;
#endif
    
    LandscapeModel::LandscapeModel  lmodel(terrainZLength, terrainXWidth, terrainMaxYHeight, roadWidth, roadPivotWidthX, segmentSteps, roadStepsScale, mtnSegmentLength);
    
    std::vector<std::shared_ptr<MeshObject> > myMeshObjects;
    myMeshObjects.push_back(lmodel.GetRoad());
    myMeshObjects.push_back(lmodel.GetTerrain());
    myMeshObjects.push_back(lmodel.GetMountain());
    
    for (size_t i = 0; i < myMeshObjects.size(); ++i) {
        myMeshObjects[i]->SetMaterial(cubeMaterial->Clone());
    }
    
    std::shared_ptr<SceneObject> mySceneObject = std::make_shared<SceneObject>();
    mySceneObject->AddMeshObject(myMeshObjects);
    mySceneObject->CreateAccelerationData(AccelerationTypes::NONE);
    mySceneObject->SetPosition(glm::vec3(-1.0, 0.0, 1.0));
    newScene->AddSceneObject(mySceneObject);
#endif
    
#if 1
    // Sky model
    
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> skyObjects = MeshLoader::LoadMesh("local/sky/skySphere_old.obj", &loadedMaterials);
    for (size_t i = 0; i < skyObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        skyObjects[i]->SetMaterial(materialCopy);
    }
    
    std::shared_ptr<SceneObject> skySceneObject = std::make_shared<SceneObject>();
    skySceneObject->AddMeshObject(skyObjects);
    skySceneObject->CreateAccelerationData(AccelerationTypes::NONE);
    newScene->AddSceneObject(skySceneObject);
#endif
    
#if 0
    // A car model
    
    std::vector<std::shared_ptr<aiMaterial>> carLoadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> carObjects = MeshLoader::LoadMesh("local/car/2003eclipse.obj", &carLoadedMaterials);
    for (size_t i = 0; i < carObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(carLoadedMaterials[i]);
        carObjects[i]->SetMaterial(materialCopy);
    }
    
    std::shared_ptr<SceneObject> carSceneObject = std::make_shared<SceneObject>();
    carSceneObject->AddMeshObject(carObjects);
    carSceneObject->CreateAccelerationData(AccelerationTypes::NONE);
    carSceneObject->MultScale(0.05);
    newScene->AddSceneObject(carSceneObject);
#endif

    
#endif
    
    // Lights
    
#if 1
    std::shared_ptr<Light> dirLight = std::make_shared<DirectionalLight>();
    dirLight->Rotate(glm::vec3(SceneObject::GetWorldRight()), -PI / 6.f);
    dirLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(dirLight);
#endif
    
#if 1
    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(pointLight);
#endif
    
    return newScene;
    
}
std::shared_ptr<ColorSampler> Scanline::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    // ASSIGNMENT 5 TODO: Change the grid size to be glm::ivec3(X, Y, 1).
    jitter->SetGridSize(glm::ivec3(1, 1, 1));
    
    std::shared_ptr<SimpleAdaptiveSampler> sampler = std::make_shared<SimpleAdaptiveSampler>();
    sampler->SetInternalSampler(jitter);
    
    // ASSIGNMENT 5 TODO: Change the '1.f' in '1.f * SMALL_EPSILON' here to be higher and see what your results are. (Part 3)
    sampler->SetEarlyExitParameters(1.f * SMALL_EPSILON, 4);
    
    // ASSIGNMENT 5 TODO: Comment out 'return jitter;' to use the adaptive sampler. (Part 2)
    return jitter;
    return sampler;
}

std::shared_ptr<class Renderer> Scanline::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    return std::make_shared<BackwardRenderer>(scene, sampler);
}

int Scanline::GetSamplesPerPixel() const
{
    // ASSIGNMENT 5 TODO: Change the '1' here to increase the maximum number of samples used per pixel. (Part 1).
    return 1;
}

bool Scanline::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int Scanline::GetMaxReflectionBounces() const
{
    return 0;
}

int Scanline::GetMaxRefractionBounces() const
{
    return 0;
}

glm::vec2 Scanline::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}