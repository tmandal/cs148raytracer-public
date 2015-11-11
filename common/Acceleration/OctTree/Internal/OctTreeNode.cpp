#include "common/Acceleration/OctTree/Internal/OctTreeNode.h"
#include "common/Acceleration/AccelerationNode.h"
#include "common/Intersection/IntersectionState.h"

// Utility function to check intersection between two boxes
bool Intersect(const Box& box1, const Box& box2)
{
    for (int i = 0; i < 3; ++i)
        if (box1.minVertex[i] < box2.maxVertex[i] && box2.minVertex[i] < box1.maxVertex[i])
            return true;
    return false;
}

OctTreeNode::OctTreeNode(std::vector<std::shared_ptr<AccelerationNode>>& childObjects, const Box& boundingBox, int maximumChildren, int maximumDepth, int depth):
isLeafNode(false), boundingBox(boundingBox)
{
    CreateParentNode(childObjects, maximumChildren, maximumDepth, depth);
}

void OctTreeNode::CreateLeafNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects)
{
    isLeafNode = true;
    leafNodes.insert(leafNodes.end(), childObjects.begin(), childObjects.end());
}

void OctTreeNode::CreateParentNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int maximumDepth, int depth)
{
    // Stop criteria
    if (childObjects.size() < (size_t)maximumChildren || depth >= maximumDepth)  {
        CreateLeafNode(childObjects);
        return;
    }
    
    // Equally divide the bounding box into 8 octants
    glm::vec3   bboxMin = boundingBox.minVertex;
    glm::vec3   bboxCenter = boundingBox.Center();
    glm::vec3   bboxMax = boundingBox.maxVertex;
    glm::vec3   bboxHLen = bboxCenter - bboxMin;
    glm::vec3   bboxHLenY = glm::vec3(0.0, bboxHLen.y, 0.0);
    glm::vec3   bboxHLenZ = glm::vec3(0.0, 0.0, bboxHLen.z);
    glm::vec3   bboxHLenYZ = glm::vec3(0.0, bboxHLen.y, bboxHLen.z);
    
    std::vector<Box>    octantBoxes;
    octantBoxes.resize(8);
    octantBoxes.emplace_back(bboxMin,              bboxCenter);
    octantBoxes.emplace_back(bboxMin + bboxHLenZ,  bboxCenter + bboxHLenZ);
    octantBoxes.emplace_back(bboxMin + bboxHLenY,  bboxCenter + bboxHLenY);
    octantBoxes.emplace_back(bboxMin + bboxHLenYZ, bboxCenter + bboxHLenYZ);
    octantBoxes.emplace_back(bboxCenter - bboxHLenYZ, bboxMax - bboxHLenYZ);
    octantBoxes.emplace_back(bboxCenter - bboxHLenY,  bboxMax - bboxHLenY);
    octantBoxes.emplace_back(bboxCenter - bboxHLenZ,  bboxMax - bboxHLenZ);
    octantBoxes.emplace_back(bboxCenter,              bboxMax);
    
    // Distribute objects in 8 octants. An object can be in multiple octants.
    std::vector<std::vector<std::shared_ptr<class AccelerationNode>>>   octantObjects;
    octantObjects.resize(8);
    
    for (size_t i = 0; i < childObjects.size(); ++i)
    {
        for (size_t o = 0; o < 8; ++o)
        {
            if (Intersect(octantBoxes[o], childObjects[i]->GetBoundingBox()))
                octantObjects[o].push_back(childObjects[i]);
        }
    }
    
    // Recursively create child nodes
    childOctTreeNodes.resize(8);
    
    for (size_t o = 0; o < 8; ++o)
    {
        if (octantObjects[o].size() == 0)
            childOctTreeNodes[o] = nullptr;
        else
            childOctTreeNodes[o] = std::make_shared<OctTreeNode>(octantObjects[o], octantBoxes[o], maximumChildren, maximumDepth, depth+1);
    }
}

bool OctTreeNode::Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const
{
    if (!boundingBox.Trace(parentObject, inputRay, outputIntersection)) {
        return false;
    }
    
    bool hitObject = false;
    // Merge these two branches....
    if (isLeafNode) {
        for (size_t i = 0; i < leafNodes.size(); ++i) {
            hitObject |= leafNodes[i]->Trace(parentObject, inputRay, outputIntersection);
        }
    } else {
        for (size_t i = 0; i < childOctTreeNodes.size(); ++i) {
            hitObject |= childOctTreeNodes[i]->Trace(parentObject, inputRay, outputIntersection);
        }
    }
    return hitObject;
}

std::string OctTreeNode::PrintContents() const
{
    std::ostringstream ss;
    if (isLeafNode) {
        for (size_t i = 0; i < leafNodes.size(); ++i) {
            ss << leafNodes[i]->GetHumanIdentifier() << "  ";
        }
    } else {
        for (size_t i = 0; i < childOctTreeNodes.size(); ++i) {
            ss << childOctTreeNodes[i]->PrintContents() << "  ";
        }
    }
    return ss.str();
}
