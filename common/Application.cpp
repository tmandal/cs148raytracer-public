#include "common/Application.h"
#include "common/Acceleration/AccelerationCommon.h"
#include "common/Output/ImageWriter.h"

std::string Application::GetOutputFilename() const
{
    return outputFilename;
}

int Application::GetSamplesPerPixel() const
{
    return 16;
}

void Application::SetImageOutputResolution(glm::vec2 resolution)
{
    imageResolution = resolution;
}

void Application::SetImageGridIndex(glm::uvec2 gridIndex)
{
    imageGridIndex = gridIndex;
}

void Application::SetImageGridSize(glm::uvec2 gridSize)
{
    imageGridSize = gridSize;
}

glm::vec2 Application::GetImageOutputResolution() const
{
    return imageResolution;
}

glm::uvec2 Application::GetImageGridIndex() const
{
    return  imageGridIndex;
}

glm::uvec2 Application::GetImageGridSize() const
{
    return  imageGridSize;
}

void Application::PerformImagePostprocessing(class ImageWriter&)
{
}
