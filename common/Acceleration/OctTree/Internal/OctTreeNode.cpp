#include "common/Acceleration/OctTree/Internal/OctTreeNode.h"
#include "common/Acceleration/AccelerationNode.h"
#include "common/Intersection/IntersectionState.h"

// Utility function
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
    leafObjects.insert(leafObjects.end(), childObjects.begin(), childObjects.end());
}

void OctTreeNode::CreateParentNode(std::vector<std::shared_ptr<class AccelerationNode>>& childObjects, int maximumChildren, int maximumDepth, int depth)
{
    // Stop criteria
    if (childObjects.size() < maximumChildren || depth < maximumDepth)  {
        CreateLeafNode(childObjects);
        return;
    }
    
    // Equally divide the bounding box into 8 octants
    glm::vec3   bboxMin = boundingBox.minVertex;
    glm::vec3   bboxCenter = boundingBox.Center();
    glm::vec3   bboxMax = boundingBox.maxVertex;
    glm::vec3   bboxHLen = bboxCenter - bboxMin;
    
    std::vector<Box>    octantBoxes;
    octantBoxes.resize(8);
    octantBoxes.emplace_back(bboxMin, bboxCenter);
    octantBoxes.emplace_back(bboxMin + glm::vec3(0.0, 0.0, bboxHLen.z), bboxCenter + glm::vec3(0.0, 0.0, bboxHLen.z));
    octantBoxes.emplace_back(bboxMin + glm::vec3(0.0, bboxHLen.y, 0.0), bboxCenter + glm::vec3(0.0, bboxHLen.y, 0.0));
    octantBoxes.emplace_back(bboxMin + glm::vec3(0.0, bboxHLen.y, bboxHLen.z), bboxCenter + glm::vec3(0.0, bboxHLen.y, bboxHLen.z));
    octantBoxes.emplace_back(bboxCenter + glm::vec3(0.0, -bboxHLen.y, -bboxHLen.z), bboxMax + glm::vec3(0.0, -bboxHLen.y, -bboxHLen.z));
    octantBoxes.emplace_back(bboxCenter + glm::vec3(0.0, -bboxHLen.y, 0.0), bboxMax + glm::vec3(0.0, -bboxHLen.y, 0.0));
    octantBoxes.emplace_back(bboxCenter + glm::vec3(0.0, 0.0, -bboxHLen.z), bboxMax + glm::vec3(0.0, 0.0, -bboxHLen.z));
    octantBoxes.emplace_back(bboxCenter, bboxMax);
    
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
    
    childNodes.resize(8);
    
    for (size_t o = 0; o < 8; ++o)
    {
        if (octantObjects[o].size() == 0)
        {
            childNodes[o] = nullptr;
        }
        else
        {
            childNodes[o] = std::make_shared<OctTreeNode>(octantObjects[o], octantBoxes[o], maximumChildren, maximumDepth, depth+1);
        }
    }
}

bool OctTreeNode::Trace(const class SceneObject* parentObject, class Ray* inputRay, struct IntersectionState* outputIntersection) const
{
    float previousIntersectionT = outputIntersection ? outputIntersection->intersectionT : 0.f;
    if (!boundingBox.Trace(parentObject, inputRay, outputIntersection)) {
        if (outputIntersection) {
            outputIntersection->intersectionT = previousIntersectionT;
        }
        return false;
    }
    
    if (outputIntersection) {
        outputIntersection->intersectionT = previousIntersectionT;
    }
    
    bool hitObject = false;
    // Merge these two branches....
    if (isLeafNode) {
        for (size_t i = 0; i < leafNodes.size(); ++i) {
            hitObject |= leafNodes[i]->Trace(parentObject, inputRay, outputIntersection);
        }
    } else {
        for (size_t i = 0; i < childBVHNodes.size(); ++i) {
            hitObject |= childBVHNodes[i]->Trace(parentObject, inputRay, outputIntersection);
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
        for (size_t i = 0; i < childBVHNodes.size(); ++i) {
            ss << childBVHNodes[i]->PrintContents() << "  ";
        }
    }
    return ss.str();
}
