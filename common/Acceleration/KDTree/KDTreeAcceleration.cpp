#include "common/Acceleration/KDTree/KDTreeAcceleration.h"
#include "common/Acceleration/KDTree/Internal/KDNode.h"
#include "common/Scene/Geometry/Ray/Ray.h"
#include "common/Intersection/IntersectionState.h"

KDTreeAcceleration::KDTreeAcceleration():
    maximumChildren(2)
{
}

bool KDTreeAcceleration::Trace(const SceneObject* parentObject, Ray* inputRay, IntersectionState* outputIntersection) const
{
    return rootNode->Trace(parentObject, inputRay, outputIntersection);
}

void KDTreeAcceleration::InternalInitialization()
{
#if !DISABLE_ACCELERATION_CREATION_TIMER
    DIAGNOSTICS_TIMER(timer, "KDTree Creation Time");
#endif

    rootNode = std::make_shared<KDNode>(nodes, maximumChildren);
}

void KDTreeAcceleration::SetMaximumChildren(int input)
{
    maximumChildren = input;
}
