#include "common/Scene/Camera/Perspective/PerspectiveCamera.h"
#include "common/Scene/Geometry/Ray/Ray.h"
#include "common/utils.h"

PerspectiveCamera::PerspectiveCamera(float aspectRatio, float inputFov):
    aspectRatio(aspectRatio), fov(inputFov * PI / 180.f), zNear(0.f), zFar(std::numeric_limits<float>::max()), zFocal(1.0f), apertureRadius(0.0f)
{
}

std::shared_ptr<Ray> PerspectiveCamera::GenerateRayForNormalizedCoordinates(glm::vec2 coordinate) const
{
    // Send ray from the camera to the image plane -- make the assumption that the image plane is at z = 1 in camera space.
    const glm::vec3 cameraCenter = glm::vec3(GetPosition());
    glm::vec3       rayOrigin = cameraCenter;
    
    // Modify ray origin to accodomate aperture
    if (apertureRadius > LARGE_EPSILON)
    {
        float   r = RandFloat01() * apertureRadius;
        float   theta = RandFloat01() * 2.0 * PI;
        float   xShift = r * cosf(theta);
        float   yShift = r * sinf(theta);
        
        rayOrigin = rayOrigin + glm::vec3(GetRightDirection()) * xShift + glm::vec3(GetUpDirection()) * yShift;
    }

    // Figure out where the ray is supposed to point to. 
    // Imagine that a frustum exists in front of the camera (which we assume exists at a singular point).
    // Then, given the aspect ratio and vertical field of view we can determine where in the world the 
    // image plane will exist and how large it is assuming we know for sure that z = 1 (this is fairly arbitrary for now).
    const float planeHeight = std::tan(fov / 2.f) * 2.f;
    const float planeWidth = planeHeight * aspectRatio;

    // Assume that (0, 0) is the top left of the image which means that when coordinate is (0.5, 0.5) the 
    // pixel is directly in front of the camera...
    const float xOffset = planeWidth * (coordinate.x - 0.5f);
    const float yOffset = -1.f * planeHeight  * (coordinate.y - 0.5f);
    const glm::vec3 targetPosition = cameraCenter + glm::vec3(GetForwardDirection()) * zFocal + glm::vec3(GetRightDirection()) * xOffset * zFocal + glm::vec3(GetUpDirection()) * yOffset * zFocal;

    const glm::vec3 rayDirection = glm::normalize(targetPosition - rayOrigin);
    return std::make_shared<Ray>(rayOrigin + rayDirection * zNear, rayDirection, zFar - zNear);
}

void PerspectiveCamera::SetZNear(float input)
{
    zNear = input;
}

void PerspectiveCamera::SetZFar(float input)
{
    zFar = input;
}

void PerspectiveCamera::SetZFocal(float input)
{
    zFocal = input;
}

void PerspectiveCamera::SetApertureRadius(float input)
{
    apertureRadius = input;
}