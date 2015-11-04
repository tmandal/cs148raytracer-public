#include "common/Rendering/Material/BlinnPhong/BlinnPhongMaterial.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/Lights/Light.h"
#include "assimp/material.h"

BlinnPhongMaterial::BlinnPhongMaterial():
    shininess(0.f)
{
}

void BlinnPhongMaterial::SetDiffuse(glm::vec3 input)
{
    diffuseColor = input;
}

void BlinnPhongMaterial::SetSpecular(glm::vec3 inputColor, float inputShininess)
{
    specularColor = inputColor;
    shininess = inputShininess;
}

glm::vec3 BlinnPhongMaterial::ComputeDiffuse(const IntersectionState& intersection, const glm::vec3& lightColor, const float NdL, const float NdH, const float NdV, const float VdH) const
{
    const float d = NdL;
    const glm::vec3 diffuseResponse = d * diffuseColor * lightColor;
    return diffuseResponse;
}

glm::vec3 BlinnPhongMaterial::ComputeSpecular(const IntersectionState& intersection, const glm::vec3& lightColor, const float NdL, const float NdH, const float NdV, const float VdH) const
{
    const float highlight = std::pow(NdH, shininess);
    const glm::vec3 specularResponse = highlight * specularColor * lightColor;
    return specularResponse;
}

std::shared_ptr<Material> BlinnPhongMaterial::Clone() const
{
    return std::make_shared<BlinnPhongMaterial>(*this);
}

void BlinnPhongMaterial::LoadMaterialFromAssimp(std::shared_ptr<aiMaterial> assimpMaterial)
{
    if (!assimpMaterial) {
        return;
    }

    Material::LoadMaterialFromAssimp(assimpMaterial);

    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, glm::value_ptr(diffuseColor), nullptr);
    assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, glm::value_ptr(specularColor), nullptr);
    assimpMaterial->Get(AI_MATKEY_SHININESS, &shininess, nullptr);

    if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE)) {
        aiString aiDiffusePath;
        assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiDiffusePath);
        std::string diffusePath(aiDiffusePath.C_Str());
        (void)diffusePath;
    }

    if (assimpMaterial->GetTextureCount(aiTextureType_SPECULAR)) {
        aiString aiSpecularPath;
        assimpMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiSpecularPath);
        std::string specularPath(aiSpecularPath.C_Str());
        (void)specularPath;
    }

}