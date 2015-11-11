#pragma once

#include "common/common.h"
#include "common/Scene/Geometry/Simple/Box/Box.h"

class OctTreeNode : public std::enable_shared_from_this <OctTreeNode>
{
public:
    OctTreeNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, const Box& boundingBox, int maximumChildren, int maximumDepth, int depth=0);
    bool Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const;
private:
    void CreateLeafNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects);
    void CreateParentNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int maximumDepth, int depth);
    std::string PrintContents() const;
    
    std::vector<std::shared_ptr<OctTreeNode>>               childOctTreeNodes;
    std::vector<std::shared_ptr<class AccelerationNode>>    leafNodes;
    bool isLeafNode;
    Box boundingBox;
};
