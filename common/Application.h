#pragma once

#include "common/common.h"

enum class AccelerationTypes;

class Application : public std::enable_shared_from_this<Application>
{
public:
    Application()
            : outputFilename("output.png"),
            imageResolution(glm::vec2(1280.f, 720.f)),
            imageGridIndex(glm::uvec2(0, 0)),
            imageGridSize(glm::uvec2(720, 1280))
    {}
    virtual ~Application() {}
    virtual std::shared_ptr<class Camera> CreateCamera() const = 0;
    virtual std::shared_ptr<class Scene> CreateScene() const = 0;
    virtual std::shared_ptr<class ColorSampler> CreateSampler() const = 0;
    virtual std::shared_ptr<class Renderer> CreateRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler) const = 0;

    // ray tracing properties
    virtual int GetMaxReflectionBounces() const = 0;
    virtual int GetMaxRefractionBounces() const = 0;

    // output
    void SetImageOutputResolution(glm::vec2 resolution);
    void SetImageGridIndex(glm::uvec2 gridIndex);
    void SetImageGridSize(glm::uvec2 gridSize);
    virtual glm::vec2 GetImageOutputResolution() const;
    virtual glm::uvec2 GetImageGridIndex() const;
    virtual glm::uvec2 GetImageGridSize() const;

    // Sampling Properties
    virtual int GetSamplesPerPixel() const;

    // whether or not to continue sampling the scene from the camera.
    virtual bool NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex) = 0;

    // Postprocessing
    virtual void PerformImagePostprocessing(class ImageWriter& imageWriter);
    virtual void SetOutputFilename(std::string filename)    { outputFilename = filename; }
    virtual std::string GetOutputFilename() const;
private:
    std::string outputFilename;
    glm::vec2   imageResolution;
    glm::uvec2  imageGridIndex;
    glm::uvec2  imageGridSize;
};
