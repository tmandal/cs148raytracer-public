#pragma once

#include "common/Acceleration/AccelerationStructure.h"

class OctTreeAcceleration : public AccelerationStructure
{
public:
    OctTreeAcceleration();
    virtual bool Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const override;
    
    void SetMaximumChildren(int input);
    void SetMaximumDepth(int input);
    
private:
    virtual void InternalInitialization() override;
    
    int maximumChildren;
    int maximumDepth;
    
    std::shared_ptr<class OctTreeNode> rootNode;
};