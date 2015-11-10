#pragma once

#include "common/common.h"
#include "common/Scene/Geometry/Simple/Box/Box.h"

class KDNode : public std::enable_shared_from_this <KDNode>
{
public:
    KDNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int depth=0, int splitDim = 0);
    bool Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const;
private:
    void CreateLeafNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects);
    void CreateNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int depth, int splitDim);
    std::string PrintContents() const;

    glm::vec3   splitPoint;
    int         splitAxis;

    std::shared_ptr<KDNode> leftNode;
    std::shared_ptr<KDNode> rightNode;
    std::vector<std::shared_ptr<class AccelerationNode>> leafObjects;
    bool isLeafNode;
    Box boundingBox;
};
