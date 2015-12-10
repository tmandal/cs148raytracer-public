#include "rtimage/RtImage.h"
#include "common/core.h"
#include "assimp/material.h"


std::shared_ptr<Camera> RtImage::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
#if 1
    std::shared_ptr<PerspectiveCamera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    //camera->SetPosition(glm::vec3(0.5f, -4.f, 2.f));
    camera->SetPosition(glm::vec3(0.25f, -4.f, -0.5f));
#else
    std::shared_ptr<PerspectiveCamera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 58.f);
    camera->SetPosition(glm::vec3(0.5f, -8.f, 2.f));
#endif
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.0f/* - PI/12.f*/);
    
    // For depth of field
    //camera->SetZFocal(3.5);
    //camera->SetApertureRadius(0.2);

    return camera;
}

std::shared_ptr<Scene> RtImage::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> defaultMaterial = std::make_shared<BlinnPhongMaterial>();
    defaultMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    defaultMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    //defaultMaterial->SetReflectivity(0.001);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> rtMaterials;
    std::vector<std::shared_ptr<MeshObject>> rtObjects;
    
    rtObjects = MeshLoader::LoadMesh("rtimage/RT2.obj", &rtMaterials);
    
    for (size_t i = 0; i < rtObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = defaultMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(rtMaterials[i]);
        
        aiString matName;
        rtMaterials[i]->Get(AI_MATKEY_NAME, matName);
        std::string matNameStr = std::string(matName.C_Str());
        
        if (matNameStr.compare("FloorSG") == 0)
        {
            materialCopy->SetReflectivity(0.1);
            materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("rtimage/Marble.jpg"));
            materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("rtimage/Marble.jpg"));
            materialCopy->SetDiffuseAttenuation(0.5f);
        }
        else if (matNameStr.compare("WindowFrameSG") == 0)
        {
            materialCopy->SetReflectivity(0.1);
            materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("rtimage/WindowFrameWood.jpg"));
            materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("rtimage/WindowFrameWood.jpg"));
        }
        else if (matNameStr.compare("WindowGlassSG") == 0)
        {
            materialCopy->SetReflectivity(0.2);
            materialCopy->SetTransmittance(0.8);
        }
        else if (matNameStr.compare("WineGlassSG") == 0)
        {
            materialCopy->SetReflectivity(0.1);
            materialCopy->SetTransmittance(0.9);
        }
        else if (matNameStr.compare("WineLiquidSG") == 0)
        {
            materialCopy->SetTransmittance(0.5);
        }
        else if (matNameStr.compare("WindowHandleSG") == 0)
        {
            materialCopy->SetReflectivity(0.5);
        }
        else if (matNameStr.compare("TableSG") == 0)
        {
            materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("rtimage/WindowFrameWood.jpg"));
            materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("rtimage/WindowFrameWood.jpg"));
        }
        else if (matNameStr.compare("FrontWallSG") == 0)
        {
            materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("rtimage/Wallpaper1.jpg"));
            materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("rtimage/Wallpaper1.jpg"));
            materialCopy->SetDiffuseAttenuation(0.5f);
            rtObjects[i]->ScalePrimitiveUV(glm::vec2(8.0, 8.0));
        }
        else if (matNameStr.compare("CityscapeSG") == 0)
        {
            materialCopy->SetTexture("ambientTexture", TextureLoader::LoadTexture("rtimage/Cityscape3.jpg"));
        }
        else if (matNameStr.compare("WallLight:CrystalSG") == 0)
        {
            materialCopy->SetTransmittance(0.9);
        }
        else if (matNameStr.compare("WallLight:MetalRingSG") == 0)
        {
            materialCopy->SetReflectivity(0.5);
        }
        else if (matNameStr.compare("WallLight:MetalFrameSG") == 0)
        {
            materialCopy->SetReflectivity(0.5);
        }
        else if (matNameStr.compare("WallLight:LightSG") == 0)
        {
        }
        else if (matNameStr.compare("WallLight:LightShadeSG") == 0)
        {
            materialCopy->SetTransmittance(0.7);
            materialCopy->SetReflectivity(0.2);
        }
        else if (matNameStr.compare("XmasLightSG") == 0)
        {
            //materialCopy->SetTransmittance(0.05);
        }
        
        rtObjects[i]->SetMaterial(materialCopy);
        
        std::cout << "RT2 : Mesh object index - " << i << " name - " << rtObjects[i]->GetName() << " , material name - " << matNameStr << std::endl;
    }
    
    //rtObjects.erase(rtObjects.begin()+13);
    //rtObjects.erase(rtObjects.begin()+8);
    //rtObjects.erase(rtObjects.begin()+0);
    

    std::shared_ptr<SceneObject> rtSceneObject = std::make_shared<SceneObject>();
    rtSceneObject->AddMeshObject(rtObjects);
    rtSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    rtSceneObject->MultScale(1/100.f);
    rtSceneObject->Translate(glm::vec3(0.f, 5.f, 0.f));
    rtSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
    newScene->AddSceneObject(rtSceneObject);
    

    // Lights
#if 0
    std::shared_ptr<PointLight> ceilPointLight = std::make_shared<PointLight>();
    ceilPointLight->SetPosition(glm::vec3(3.0f, 0.0f, 4.80f));
    ceilPointLight->SetLightColor(glm::vec3(0.5f));
    newScene->AddLight(ceilPointLight);
#endif

#if 0
    std::shared_ptr<AreaLight> ceilAreaLight = std::make_shared<AreaLight>(glm::vec2(0.5f, 0.5f));
    ceilAreaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 8);
    ceilAreaLight->SetPosition(glm::vec3(3.0f, 0.0f, 4.80f));
    ceilAreaLight->SetLightColor(glm::vec3(0.5f));
    newScene->AddLight(ceilAreaLight);
#endif

#if 0
    std::shared_ptr<PointLight> leftXmasLight = std::make_shared<PointLight>();
    leftXmasLight->SetPosition(glm::vec3(0.25f, 2.03f, -1.13));
    leftXmasLight->SetLightColor(glm::vec3(1.0f));
    newScene->AddLight(leftXmasLight);
#endif

#if 0
    std::shared_ptr<PointLight> rightXmasLight = std::make_shared<PointLight>();
    rightXmasLight->SetPosition(glm::vec3(1.56f, 2.03f, -1.13f));
    rightXmasLight->SetLightColor(glm::vec3(1.0f));
    newScene->AddLight(rightXmasLight);
#endif

#if 1
    glm::vec3   externLightCenter = glm::vec3(0.8951965, 5.7, 0.4052295);
    //glm::vec3   externLightSize = glm::vec3(1.9, 0.0, 1.4);
    glm::vec3   externLightSize = glm::vec3(1.0, 0.0, 1.0);
    std::vector<glm::vec3>   externLightColor;
    externLightColor.push_back(glm::vec3(249, 149, 51)/255.f);  // 0
    externLightColor.push_back(glm::vec3(242, 211, 57)/255.f);  // 1
    externLightColor.push_back(glm::vec3(198, 77, 112)/255.f);  // 2
    externLightColor.push_back(glm::vec3(159, 211, 246)/255.f);  // 3
    externLightColor.push_back(glm::vec3(0)/255.f);  // 4 -skipped
    externLightColor.push_back(glm::vec3(250, 162, 63)/255.f);  // 5
    externLightColor.push_back(glm::vec3(174, 55, 64)/255.f);  // 6
    externLightColor.push_back(glm::vec3(114, 109, 192)/255.f);  // 7
    externLightColor.push_back(glm::vec3(94, 126, 223)/255.f);  // 8
    for (int i = -1; i <= 1; ++i)
    {
        for (int k = -1; k <= 1; ++k)
        {
            if (i == 0 && k == 0)
                continue;
            size_t      externLightIndex = (i+1) * 3 + (k+1);
            glm::vec3   pointLightPosition = externLightCenter + glm::vec3(i, 0, k) * externLightSize/2.f;
            std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(pointLightPosition);
            pointLight->SetLightColor(externLightColor[externLightIndex]);
            newScene->AddLight(pointLight);
        }
    }
#endif

#if 0
    std::shared_ptr<VolumeLight> volumeLight = std::make_shared<VolumeLight>(8);
    volumeLight->AddMeshObject(std::vector<std::shared_ptr<MeshObject>>(1, rtObjects[20])); // LeftLight
    volumeLight->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    volumeLight->MultScale(1/100.f);
    volumeLight->Translate(glm::vec3(0.f, 5.f, 0.f));
    volumeLight->CreateAccelerationData(AccelerationTypes::BVH);
    volumeLight->Finalize();
    volumeLight->SetLightColor(glm::vec3(1.f));
    newScene->AddLight(volumeLight);
    
    std::shared_ptr<VolumeLight> volumeLight2 = std::make_shared<VolumeLight>(8);
    volumeLight2->AddMeshObject(std::vector<std::shared_ptr<MeshObject>>(1, rtObjects[42])); // RightLight
    volumeLight2->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    volumeLight2->MultScale(1/100.f);
    volumeLight2->Translate(glm::vec3(0.f, 5.f, 0.f));
    volumeLight2->CreateAccelerationData(AccelerationTypes::BVH);
    volumeLight2->Finalize();
    volumeLight2->SetLightColor(glm::vec3(1.f));
    newScene->AddLight(volumeLight2);
#endif
    
#if 0
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(1.0f, 1.0f));
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 8);
    areaLight->SetPosition(glm::vec3(0.0f, 0.0f, 6.0f));
    areaLight->SetLightColor(glm::vec3(244.f/255.f, 255.f/255.f, 250.f/255.f));
    newScene->AddLight(areaLight);
#endif
    
#if 0
    std::shared_ptr<AreaLight> areaLight2 = std::make_shared<AreaLight>(glm::vec2(3.0f, 3.0f));
    areaLight2->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 8);
    areaLight2->SetPosition(glm::vec3(0.77f, 5.5f, 0.18f));
    areaLight2->Rotate(glm::vec3(1.f, 0.f, 0.f), -PI/2.f);
    areaLight2->SetLightColor(glm::vec3(1.f));
    newScene->AddLight(areaLight2);
#endif

#if 0
    std::shared_ptr<DirectionalAreaLight> directionalAreaLight = std::make_shared<DirectionalAreaLight>(glm::vec2(3.0f, 3.0f), glm::vec3(0.0f, 1.0f, -2.0f));
    directionalAreaLight->SetPosition(glm::vec3(0.77f, 5.5f, 0.18f));
    directionalAreaLight->Rotate(glm::vec3(1.f, 0.f, 0.f), -PI/2.f);
    directionalAreaLight->SetLightColor(glm::vec3(1.f));
    newScene->AddLight(directionalAreaLight);
#endif

    return newScene;

}
std::shared_ptr<ColorSampler> RtImage::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(glm::ivec3(1, 1, 1));
    return jitter;
}

std::shared_ptr<class Renderer> RtImage::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
#if 1
    return std::make_shared<BackwardRenderer>(scene, sampler);
#else
    std::shared_ptr<class PhotonMappingRenderer>    photonRenderer = std::make_shared<PhotonMappingRenderer>(scene, sampler);
    //photonRenderer->SetNumberOfDiffusePhotons(100000);
    //photonRenderer->SetNumberOfSpecularPhotons(50000);
    photonRenderer->SetDiffusePhotonSphereRadius(0.03);
    photonRenderer->SetDiffusePhotonGatherMultiplier(16.0);
    photonRenderer->SetSpecularPhotonSphereRadius(0.01);
    photonRenderer->SetSpecularPhotonGatherMultiplier(16.0);
    return photonRenderer;
#endif
}

int RtImage::GetSamplesPerPixel() const
{
    return 1;
}

bool RtImage::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int RtImage::GetMaxReflectionBounces() const
{
    return 2;
}

int RtImage::GetMaxRefractionBounces() const
{
    return 6;
}

glm::vec2 RtImage::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
