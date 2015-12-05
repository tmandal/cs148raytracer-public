//
//
//

#include "common/Rendering/Media/Media.h"

template <typename T>
T TrilinearInterpolation(glm::vec3 v, T* p)
{
    T&  p000 = p[0];
    T&  p001 = p[1];
    T&  p010 = p[2];
    T&  p011 = p[3];
    T&  p100 = p[4];
    T&  p101 = p[5];
    T&  p110 = p[6];
    T&  p111 = p[7];
    
    T   px00 = p000 * v.x + p100 * (1-v.x);
    T   px01 = p001 * v.x + p101 * (1-v.x);
    T   px10 = p010 * v.x + p110 * (1-v.x);
    T   px11 = p011 * v.x + p111 * (1-v.x);
    
    T   pxy0 = px00 * v.y + px10 * (1-v.y);
    T   pxy1 = px01 * v.y + px11 * (1-v.y);
    
    T   pxyz = pxy0 * v.z + pxy1 * (1-v.z);
    
    return  pxyz;
}

Media::Media(glm::vec3 voxelGridSize, float spacing)
{
}

Media::~Media()
{
    
}

float Media::ComputeLightAttenuation(const struct IntersectionState* xState) const
{
    assert(xState->hasIntersection && xState->intersectedPrimitive->GetParentMeshObject()->IsMedia());
    // Compute x0 and xN for ray intersection with voxel grid
    // Translate them into object space
    // March from x0 to xN for deltaX stepsize to calculate attenuation
    assert(false);
    return 1.0;
}
