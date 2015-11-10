#include "common/Acceleration/KDTree/Internal/KDNode.h"
#include "common/Acceleration/AccelerationNode.h"
#include "common/Intersection/IntersectionState.h"

KDNode::KDNode(std::vector<std::shared_ptr<AccelerationNode>>& childObjects, int maximumChildren, int depth, int splitDim):
    leftNode(nullptr),
    rightNode(nullptr),
    isLeafNode(false)
{
    CreateNode(childObjects, maximumChildren, depth, splitDim);
}

void KDNode::CreateLeafNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects)
{
    isLeafNode = true;
    leafObjects.insert(leafObjects.end(), childObjects.begin(), childObjects.end());
    for (size_t i = 0; i < leafObjects.size(); ++i) {
        boundingBox.IncludeBox(leafObjects[i]->GetBoundingBox());
    }
}

void KDNode::CreateNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int depth, int splitDim)
{
    // Stop criteria
    if (static_cast<int>(childObjects.size()) <= maximumChildren)   {
        CreateLeafNode(childObjects);
        return;
    }

    // Find avg center of all nodes and split around the chosen axis around it
    splitPoint = glm::vec3(0.0f);
    for (size_t i = 0; i < childObjects.size(); ++i) {
        splitPoint += childObjects[i]->GetBoundingBox().Center();
#if 0
        std::cout << "    Object center : " << glm::to_string(childObjects[i]->GetBoundingBox().Center()) << std::endl;
#endif
    }
    splitPoint = splitPoint / (1.0f * childObjects.size());
    splitAxis = splitDim;

    const int nextDim = (splitDim + 1) % 3;

    // Split the nodes around the chosen axis at split point
    std::vector<std::shared_ptr<class AccelerationNode>>    leftObjects;
    std::vector<std::shared_ptr<class AccelerationNode>>    rightObjects;

    for (size_t i = 0; i < childObjects.size(); ++i) {
        if (childObjects[i]->GetBoundingBox().Center()[splitAxis] < splitPoint[splitAxis])
            leftObjects.push_back(childObjects[i]);
        else
            rightObjects.push_back(childObjects[i]);
    }
#if 0
    size_t  numObjects = childObjects.size();
    size_t  numLeftObjects = leftObjects.size();
    size_t  numRightObjects = rightObjects.size();

    std::cout << "KDTree : splitAxis = " << splitAxis << " splitPoint = " << glm::to_string(splitPoint) << " Depth = " << depth << " #objects = " << numObjects << " #leftObjects = " << numLeftObjects << " #rightObjects = " << numRightObjects << std::endl;
#endif

    if (leftObjects.size() == 0 || rightObjects.size() == 0)    {
        // Early termination if there is one child.
        CreateLeafNode(childObjects);
    }
    else    {
        leftNode = std::make_shared<KDNode>(leftObjects, maximumChildren, depth+1, nextDim);
        rightNode = std::make_shared<KDNode>(rightObjects, maximumChildren, depth+1, nextDim);

        boundingBox.IncludeBox(leftNode->boundingBox);
        boundingBox.IncludeBox(rightNode->boundingBox);
    }
}

bool KDNode::Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const
{
    IntersectionState tempState;
    if (!boundingBox.Trace(parentObject, inputRay, &tempState)) {
        return false;
    }

    bool hitObject = false;
    if (isLeafNode) {
        for (size_t i = 0; i < leafObjects.size(); ++i) {
            hitObject |= leafObjects[i]->Trace(parentObject, inputRay, outputIntersection);
        }
    } else {
        // Left child
        hitObject |= leftNode->Trace(parentObject, inputRay, outputIntersection);
        // Right child
        hitObject |= rightNode->Trace(parentObject, inputRay, outputIntersection);
    }
    return hitObject;
}

std::string KDNode::PrintContents() const
{
    std::ostringstream ss;
    if (isLeafNode) {
        for (size_t i = 0; i < leafObjects.size(); ++i) {
            ss << leafObjects[i]->GetHumanIdentifier() << "  ";
        }
    } else {
        ss << " l " << leftNode->PrintContents() << "  ";
        ss << " r " << rightNode->PrintContents() << "  ";
    }
    return ss.str();
}
