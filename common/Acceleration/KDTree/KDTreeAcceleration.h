#pragma once

#include "common/Acceleration/AccelerationStructure.h"

class KDTreeAcceleration : public AccelerationStructure
{
public:
    KDTreeAcceleration();
    virtual bool Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const override;

    void SetMaximumChildren(int input);

private:
    virtual void InternalInitialization() override;

    int maximumChildren;

    std::shared_ptr<class KDNode> rootNode;
};
