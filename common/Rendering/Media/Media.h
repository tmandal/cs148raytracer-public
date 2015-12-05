#pragma once

#include "common/common.h"

class Media : public std::enable_shared_from_this<Media>
{
public:
    Media(glm::vec3 voxelGridSize, float spacing);
    virtual ~Media();

    float ComputeLightAttenuation(const struct IntersectionState* xState) const;

private:
    glm::vec3   voxelGridSize;
    glm::vec3   voxelSize;
    float       spacing;
    std::vector<std::vector<std::vector<glm::vec3>>>    density;
};
