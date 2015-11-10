#include "common/Acceleration/OctTree/OctTreeAcceleration.h"
#include "common/Acceleration/OctTree/Internal/OctTreeNode.h"
#include "common/Scene/Geometry/Ray/Ray.h"
#include "common/Intersection/IntersectionState.h"

OctTreeAcceleration::OctTreeAcceleration()
: maximumChildren(2), maximumDepth(1000000)
{
}

bool OctTreeAcceleration::Trace(const SceneObject* parentObject, Ray* inputRay, IntersectionState* outputIntersection) const
{
    return rootNode->Trace(parentObject, inputRay, outputIntersection);
}

void OctTreeAcceleration::InternalInitialization()
{
#if !DISABLE_ACCELERATION_CREATION_TIMER
    DIAGNOSTICS_TIMER(timer, "OctTree Creation Time");
#endif
    
    Box boundingBox;
    
    for (size_t i = 0; i < nodes.size(); ++i)
        boundingBox.IncludeBox(nodes[i]->GetBoundingBox());
    
    rootNode = std::make_shared<OctTreeNode>(nodes, boundingBox, maximumChildren, maximumDepth);
}

void OctTreeAcceleration::SetMaximumChildren(int input)
{
    maximumChildren = input;
}

void OctTreeAcceleration::SetMaximumDepth(int input)
{
    maximumDepth = input;
}
