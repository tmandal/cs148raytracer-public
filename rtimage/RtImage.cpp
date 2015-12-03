#include "rtimage/RtImage.h"
#include "common/core.h"
#include "assimp/material.h"


std::shared_ptr<Camera> RtImage::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<PerspectiveCamera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.0f);
    
    // For depth of field
    //camera->SetZFocal(3.5);
    //camera->SetApertureRadius(0.2);

    return camera;
}

std::shared_ptr<Scene> RtImage::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    //cubeMaterial->SetReflectivity(0.001);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects;
    
#if 1
    // CornellBox-Assignment8 has BB : (-1, -1, 0) -> (1, 1, 2)
    //cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment8.obj", &loadedMaterials);
    cubeObjects = MeshLoader::LoadMesh("rtimage/GlassWithWaterAndIce.obj", &loadedMaterials);
    // material : Ice used by ice cubes
    // material : Material.001 used by glass (and possibly internals of glass)
    // material : Material.002 used by floor plane
    // material : Material.004 used by back and right planes
    // material : Material.005 used by water

    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        
        // Make the floor reflectant
        //if (i == 17)
        //    materialCopy->SetReflectivity(0.5);
        // Make glass reflectant
        //if (i == 18)
        //    materialCopy->SetReflectivity(0.1);
        
        cubeObjects[i]->SetMaterial(materialCopy);
        
        aiString matName;
        loadedMaterials[i]->Get(AI_MATKEY_NAME, matName);
        std::cout << "GlassWithWaterAndIce : Mesh object index - " << i << " name - " << cubeObjects[i]->GetName() << " , material name - " << std::string(matName.C_Str()) << std::endl;
    }
    
    cubeObjects.erase(cubeObjects.begin() + 20);

#if 0
    cubeObjects.erase(cubeObjects.begin() + 19);    // water flow
#endif

#if 0
    cubeObjects.erase(cubeObjects.begin() + 18);    // glass
#endif

#if 0
    cubeObjects.erase(cubeObjects.begin() + 14);    // ice sphere
#endif

#if 0
    cubeObjects.erase(cubeObjects.begin() + 12);    // ice cube
    cubeObjects.erase(cubeObjects.begin() + 10);    // ice cube
    cubeObjects.erase(cubeObjects.begin() +  8);    // ice cube
    cubeObjects.erase(cubeObjects.begin() +  6);    // ice cube
    cubeObjects.erase(cubeObjects.begin() +  4);    // ice cube
    cubeObjects.erase(cubeObjects.begin() +  2);    // ice cube
#endif

#if 0
    cubeObjects.erase(cubeObjects.begin() +  1);    // water splash
    cubeObjects.erase(cubeObjects.begin() +  0);    // water splash
#endif

    std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
    cubeSceneObject->AddMeshObject(cubeObjects);
    cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    cubeSceneObject->MultScale(0.3);
    cubeSceneObject->Translate(glm::vec3(-0.7f, 1.6f, 0.f));
    cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
    newScene->AddSceneObject(cubeSceneObject);
#endif
    
#if 1
    loadedMaterials.clear();
    cubeObjects = MeshLoader::LoadMesh("rtimage/candlesObj.obj", &loadedMaterials);
    
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        /*
        if (i <= 5)
            materialCopy->SetAmbient(glm::vec3(1.f, 0.f, 0.f));
        else if (i <= 10)
            materialCopy->SetAmbient(glm::vec3(0.f, 1.f, 0.f));
        else
            materialCopy->SetAmbient(glm::vec3(0.f, 0.f, 1.f));
         */
        if (i%5 == 0)   // candle stand object
            materialCopy->SetReflectivity(0.5);
        cubeObjects[i]->SetMaterial(materialCopy);
        
        aiString matName;
        loadedMaterials[i]->Get(AI_MATKEY_NAME, matName);
        std::cout << "candlesObj : Mesh object index - " << i << " name - " << cubeObjects[i]->GetName() << " , material name - " << std::string(matName.C_Str()) << std::endl;
    }
    
    cubeObjects.erase(cubeObjects.begin() + 0);
    //cubeObjects.erase(cubeObjects.begin() + 19, cubeObjects.end());
    
    for (size_t i = 0; i < 3; ++i)
    {
        std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
        size_t  candleIndex = (i == 2) ? 0 : i; // Use first candle as third candle too
        for (size_t j = 0; j < 5; ++j)
            cubeSceneObject->AddMeshObject(cubeObjects[5*candleIndex+j]);
        cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
        cubeSceneObject->MultScale(1/750.0f);
        switch(i)
        {
            case 0 :
                cubeSceneObject->Translate(glm::vec3(-2.f, 5.5f, -0.425f));
                break;
            case 1 :
                cubeSceneObject->Translate(glm::vec3( 0.f, 6.0f, -0.425f));
                break;
            case 2 :
                cubeSceneObject->Translate(glm::vec3(1.7f, 5.5f, -0.425f));
                break;
        }
        cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
        newScene->AddSceneObject(cubeSceneObject);
    }
#endif
    
#if 1
    loadedMaterials.clear();
    // CornellBox-Assignment8 has BB : (-1, -1, 0) -> (1, 1, 2)
    //cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment8.obj", &loadedMaterials);
    cubeObjects = MeshLoader::LoadMesh("rtimage/Flame.obj", &loadedMaterials);
    
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        cubeObjects[i]->SetMaterial(materialCopy);
        
        aiString matName;
        loadedMaterials[i]->Get(AI_MATKEY_NAME, matName);
        std::cout << "Flame : Mesh object index - " << i << " name - " << cubeObjects[i]->GetName() << " , material name - " << std::string(matName.C_Str()) << std::endl;
    }
    
    for (size_t i = 0; i < 3; ++i)
    {
        std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
        cubeSceneObject->AddMeshObject(cubeObjects);
        cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
        cubeSceneObject->MultScale(1/60.f);
        cubeSceneObject->Translate(glm::vec3(3.f, 0.f, -1.f));
        switch(i)
        {
            case 0 :
                cubeSceneObject->Translate(glm::vec3(-0.75f, 0.f, 0.27f));
                break;
            case 1 :
                cubeSceneObject->Translate(glm::vec3(0.175f, 0.f, 0.52f));
                break;
            case 2 :
                cubeSceneObject->Translate(glm::vec3(1.08f, 0.f, 0.27f));
                break;
        }
        
        cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
        newScene->AddSceneObject(cubeSceneObject);
    }
    
    for (size_t i = 3; i < 3; ++i)
    {
        std::shared_ptr<VolumeLight> volumeLight = std::make_shared<VolumeLight>();
        volumeLight->AddMeshObject(cubeObjects[2]);
        volumeLight->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
        volumeLight->MultScale(1/60.f);
        volumeLight->Translate(glm::vec3(3.f, 0.f, -1.f));
        switch(i)
        {
            case 0 :
                volumeLight->Translate(glm::vec3(-0.75f, 0.f, 0.27f));
                break;
            case 1 :
                volumeLight->Translate(glm::vec3(0.175f, 0.f, 0.52f));
                break;
            case 2 :
                volumeLight->Translate(glm::vec3(1.08f, 0.f, 0.27f));
                break;
        }
        volumeLight->CreateAccelerationData(AccelerationTypes::BVH);
        volumeLight->Finalize();
        volumeLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
        //volumeLight->SetLightColor(glm::vec3(226.f/255.f, 88.f/255.f, 34.f/255.f));
        volumeLight->SetNumSamples(8);
        newScene->AddLight(volumeLight);
        
        //std::cout << "Bounding box of volume light " << i << "  : minVtx = " << glm::to_string(volumeLight->GetBoundingBox().minVertex) << " maxVtx = " << glm::to_string(volumeLight->GetBoundingBox().maxVertex) << std::endl;
        
    }
#endif
    

    // Lights
#if 0
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(0.0f, 3.0f, 6.0f));
    pointLight->SetLightColor(glm::vec3(244.f/255.f, 255.f/255.f, 250.f/255.f));    // std fluorescent
    //pointLight->SetLightColor(glm::vec3(255.f/255.f, 241.f/255.f, 224.f/255.f));    // halogen
    newScene->AddLight(pointLight);
#endif
    
#if 1
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(1.0f, 1.0f));
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 8);
    areaLight->SetPosition(glm::vec3(0.0f, 0.0f, 6.0f));
    areaLight->SetLightColor(glm::vec3(244.f/255.f, 255.f/255.f, 250.f/255.f));
    newScene->AddLight(areaLight);
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
    //return std::make_shared<BackwardRenderer>(scene, sampler);
    std::shared_ptr<class PhotonMappingRenderer>    photonRenderer = std::make_shared<PhotonMappingRenderer>(scene, sampler);
    //photonRenderer->SetNumberOfDiffusePhotons(2000000);
    photonRenderer->SetPhotonSphereRadius(0.03);
    photonRenderer->SetPhotonGatherMultiplier(4.0);
    return photonRenderer;
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
    return 32;
}

glm::vec2 RtImage::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
