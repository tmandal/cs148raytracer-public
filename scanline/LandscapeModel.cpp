//
//  LandscapeModel.cpp
//  cs148-opengl4
//
//  Created by Tanmoy on 10/1/15.
//
//

#include "LandscapeModel.hpp"
#include "noise/noise.h"
#include "common/Scene/Geometry/Primitives/Triangle/Triangle.h"
#include "common/Scene/Geometry/Primitives/PrimitiveBase.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"

namespace LandscapeModel
{
    void AddTriangleIndices(const glm::uvec3& triangle, std::vector<glm::uvec3>& triangles)
    {
        triangles.push_back(triangle);
    }
    
    unsigned SimpleTriangleSubdivide(const glm::uvec3& triangle, std::vector<glm::vec3>& positions, std::vector<glm::uvec3>& triangles, unsigned subdivideLevel)
    {
        if (subdivideLevel == 0)
        {
            AddTriangleIndices(triangle, triangles);
            return 0;   // No new vertices are added
        }
        
        unsigned    aIdx = triangle[0];
        unsigned    bIdx = triangle[1];
        unsigned    cIdx = triangle[2];
        
        glm::vec3   Avtx = positions[aIdx];
        glm::vec3   Bvtx = positions[bIdx];
        glm::vec3   Cvtx = positions[cIdx];
        
        positions.emplace_back((Avtx + Bvtx) / 2.0f);
        positions.emplace_back((Bvtx + Cvtx) / 2.0f);
        positions.emplace_back((Cvtx + Avtx) / 2.0f);
        
        unsigned    abMidIdx = positions.size() - 3;
        unsigned    bcMidIdx = positions.size() - 2;
        unsigned    caMidIdx = positions.size() - 1;
        
        unsigned newVertices = 3;
        
        newVertices += SimpleTriangleSubdivide(glm::uvec3(aIdx, abMidIdx, caMidIdx), positions, triangles, subdivideLevel-1);
        newVertices += SimpleTriangleSubdivide(glm::uvec3(abMidIdx, bIdx, bcMidIdx), positions, triangles, subdivideLevel-1);
        newVertices += SimpleTriangleSubdivide(glm::uvec3(caMidIdx, bcMidIdx, cIdx), positions, triangles, subdivideLevel-1);
        newVertices += SimpleTriangleSubdivide(glm::uvec3(abMidIdx, bcMidIdx, caMidIdx), positions, triangles, subdivideLevel-1);
        
        return newVertices;
    }
    
    void AddTriangleStrip(std::vector<unsigned int>& left, std::vector<unsigned int>& right, std::vector<glm::uvec3>& triangles)
    {
        int l = 0, r = 0;
        int a, b, c, d;
        while (l < left.size() && r < right.size())
        {
            if (l < left.size()-1)
            {
                b = left[l++];
                a = left[l];
            }
            else
            {
                a = left[l];
                b = -1;
            }
            
            if (r < right.size()-1)
            {
                if (b < 0)
                {
                    b = right[r++];
                    c = right[r];
					d = -1;
                }
                else{
                    c = right[r++];
                    d = right[r];
                }
            }
            else
            {
                if (b < 0)
                {
                    b = right[r];
                    c = -1;
                }
                else{
                    c = right[r];
                    d = -1;
                }
            }
            
            if (c < 0)
                break;
            
            AddTriangleIndices(glm::uvec3(a, b, c), triangles);
            if (d >= 0)
                AddTriangleIndices(glm::uvec3(b, c, d), triangles);
        }
    }
    
    
    glm::vec2 BezierCurve(const std::vector<glm::vec2>& points, float t)
    {
        std::vector<glm::vec2>  tempPoints(points);
        for (int n = tempPoints.size()-1; n >= 0; --n)
        {
            for (int i = 0; i < n; ++i)
            {
                tempPoints[i] += t * (tempPoints[i+1] - tempPoints[i]);
            }
        }
        return tempPoints[0];
    }
    
    inline unsigned int relativeVtxIdxAt(int i, int k, int segments)
    {
        return (i * (segments + 1) + k);
    }
    
    std::unique_ptr<std::vector<glm::vec3> > genAvgVtxNormals(const std::vector<glm::vec3>& vtxPositions, const std::vector<glm::uvec3>& triangles)
    {
        std::unique_ptr<std::vector<glm::vec3> > vtxNormals = make_unique<std::vector<glm::vec3> >(vtxPositions.size(), glm::vec3(0.0, 0.0, 0.0));
        for (decltype(triangles.size()) i = 0; i < triangles.size(); i += 1) {
            unsigned aIndex = triangles[i][0];
            unsigned bIndex = triangles[i][1];
            unsigned cIndex = triangles[i][2];
            glm::vec3   normal = glm::cross(glm::vec3(vtxPositions.at(bIndex)-vtxPositions.at(aIndex)), glm::vec3(vtxPositions.at(cIndex)-vtxPositions.at(aIndex)));
            vtxNormals->at(aIndex) += normal;
            vtxNormals->at(bIndex) += normal;
            vtxNormals->at(cIndex) += normal;
        }
        
        for (decltype(vtxNormals->size()) i = 0; i < vtxNormals->size(); ++i) {
            vtxNormals->at(i) = glm::normalize(vtxNormals->at(i));
            //vtxNormals->at(i) = glm::vec3(0.0, 1.0, 0.0);
        }
        return std::move(vtxNormals);
    }

	std::unique_ptr<std::vector<glm::vec3> > genAreaWeightedAvgVtxNormals(const std::vector<glm::vec3>& vtxPositions, const std::vector<glm::uvec3>& triangles)
	{
		std::unique_ptr<std::vector<glm::vec3> > vtxNormals = make_unique<std::vector<glm::vec3> >(vtxPositions.size(), glm::vec3(0.0, 0.0, 0.0));
		glm::vec3	A;
		glm::vec3	B;
		glm::vec3	C;
		unsigned	aIndex;
		unsigned	bIndex;
		unsigned	cIndex;
		glm::vec3   normal;
		float		area;

		for (decltype(triangles.size()) i = 0; i < triangles.size(); i += 1) {
            aIndex = triangles[i][0];
            bIndex = triangles[i][1];
            cIndex = triangles[i][2];
			A = glm::vec3(vtxPositions.at(aIndex));
			B = glm::vec3(vtxPositions.at(bIndex));
			C = glm::vec3(vtxPositions.at(cIndex));

			normal = glm::cross(B-A, C-A);
			area = fabs(0.5 * glm::length(normal));

			vtxNormals->at(aIndex) += normal * area;
			vtxNormals->at(bIndex) += normal * area;
			vtxNormals->at(cIndex) += normal * area;
		}

		for (decltype(vtxNormals->size()) i = 0; i < vtxNormals->size(); ++i) {
			vtxNormals->at(i) = glm::normalize(vtxNormals->at(i));
			//vtxNormals->at(i) = glm::vec3(0.0, 1.0, 0.0);
		}
		return std::move(vtxNormals);
	}
    
    std::unique_ptr<std::vector<glm::vec3> > genAngleWeightedAvgVtxNormals(const std::vector<glm::vec3>& vtxPositions, const std::vector<glm::uvec3>& triangles)
    {
        std::unique_ptr<std::vector<glm::vec3> > vtxNormals = make_unique<std::vector<glm::vec3> >(vtxPositions.size(), glm::vec3(0.0, 0.0, 0.0));
        glm::vec3	A;
        glm::vec3	B;
        glm::vec3	C;
        unsigned	aIndex;
        unsigned	bIndex;
        unsigned	cIndex;
        glm::vec3   normal;
        float		lengthAB;
        float		lengthBC;
        float		lengthCA;
        float		angleA;
        float		angleB;
        float		angleC;
        
        for (decltype(triangles.size()) i = 0; i < triangles.size(); i += 1) {
            aIndex = triangles[i][0];
            bIndex = triangles[i][1];
            cIndex = triangles[i][2];
            A = glm::vec3(vtxPositions.at(aIndex));
            B = glm::vec3(vtxPositions.at(bIndex));
            C = glm::vec3(vtxPositions.at(cIndex));
            
            normal = glm::cross(B-A, C-A);
            
            lengthAB = glm::length(A-B);
            lengthBC = glm::length(B-C);
            lengthCA = glm::length(C-A);
            
            angleA = asin(glm::length(normal) / (lengthAB * lengthCA));
            angleB = asin(glm::length(normal) / (lengthAB * lengthBC));
            angleC = asin(glm::length(normal) / (lengthBC * lengthCA));
            
            vtxNormals->at(aIndex) += normal * angleA;
            vtxNormals->at(bIndex) += normal * angleB;
            vtxNormals->at(cIndex) += normal * angleC;
        }
        
        for (decltype(vtxNormals->size()) i = 0; i < vtxNormals->size(); ++i) {
            vtxNormals->at(i) = glm::normalize(vtxNormals->at(i));
            //vtxNormals->at(i) = glm::vec3(0.0, 1.0, 0.0);
        }
        return std::move(vtxNormals);
    }
    
    void LoadTriangleIntoPrimitive(glm::uvec3 triangle, Triangle& primitive, std::vector<glm::vec3>& allPositions, std::vector<glm::vec3>& allNormals, std::vector<glm::vec2>& allUV)
    {
        assert(3 == primitive.GetTotalVertices());
        
        for (unsigned int i = 0; i < 3; ++i) {
            primitive.SetVertexPosition(i, allPositions.at(triangle[i]));
            primitive.SetVertexNormal(i, allNormals.at(triangle[i]));
            primitive.SetVertexUV(i, allUV.at(triangle[i]));
        }
    }
    
    std::shared_ptr<MeshObject> makeMeshObject(std::unique_ptr<std::vector<glm::vec3> >     vtxPositions,
                                               std::unique_ptr<std::vector<glm::uvec3> >    triangles,
                                               std::unique_ptr<std::vector<glm::vec3> >     vtxNormals,
                                               std::unique_ptr<std::vector<glm::vec2> >     vtxUVs
                                            )
    {
        std::shared_ptr<MeshObject> mesh = std::make_shared<MeshObject>();
        
        for (decltype(triangles->size()) t = 0; t < triangles->size(); ++t)
        {
            std::shared_ptr<Triangle> primitive = std::make_shared<Triangle>(mesh.get());
            LoadTriangleIntoPrimitive(triangles->at(t), *primitive.get(), *vtxPositions, *vtxNormals, *vtxUVs);
            assert(primitive);
            mesh->AddPrimitive(primitive);
        }
        
        return  mesh;
    }
    
    LandscapeModel::LandscapeModel(float length, float width, float height, float roadWidth, float roadPivotWidthX, int steps, int roadStepsScale, float mtnSegmentLength)
    {
        
        std::unique_ptr<std::vector<glm::vec3> > roadVtxPositions = make_unique<std::vector<glm::vec3> >();
        std::unique_ptr<std::vector<glm::vec3> > terrainVtxPositions = make_unique<std::vector<glm::vec3> >();
		std::unique_ptr<std::vector<glm::vec3> > mountainVtxPositions = make_unique<std::vector<glm::vec3> >();
        std::vector<glm::uvec3> roadTriangles = std::vector<glm::uvec3>();
        std::vector<glm::uvec3> terrainTriangles = std::vector<glm::uvec3>();
		std::vector<glm::uvec3> mountainTriangles = std::vector<glm::uvec3>();
        
        // Grid configurations
        glm::uvec2		segments = glm::uvec2(steps, steps);
        glm::vec3       gridStart = glm::vec3(0.0, 0.0, 0.0);
        glm::vec2       segmentSize = glm::vec2(width/segments[0], length/segments[1]);
        
        // Road construction
        
		float	roadPivotOffsetX = width / 2.0;
        std::vector<glm::vec2>  bCtlPoints;
        bCtlPoints.emplace_back(roadPivotOffsetX,							0.0);
        bCtlPoints.emplace_back(roadPivotOffsetX - roadPivotWidthX/4.0,     -length * (1.0/4.0));
		bCtlPoints.emplace_back(roadPivotOffsetX,							-length * (2.0/4.0));
        bCtlPoints.emplace_back(roadPivotOffsetX - roadPivotWidthX/2.0,     -length * (3.0/4.0));
		bCtlPoints.emplace_back(roadPivotOffsetX - roadPivotWidthX,			-length);
        
        std::vector<glm::vec2>  bHeightPoints;
        bHeightPoints.emplace_back(0.0f, 0.0f);
        bHeightPoints.emplace_back(length/2.0f, height);
        bHeightPoints.emplace_back(length, height/2.0f);
        
        // Road segments
        std::vector<glm::uvec2>     roadSegments;
        
        glm::vec2   bPoint;
        int         bSteps = roadStepsScale * steps;
        glm::uvec2  lastLeftSeg;
        glm::uvec2  curLeftSeg;
        glm::uvec2  lastRightSeg;
        glm::uvec2  curRightSeg;
        glm::uvec2  curMinLeftSeg = glm::uvec2(segments[0]+1, segments[1]+1);
        glm::uvec2  curMaxRightSeg = glm::uvec2(0, 0);
        std::vector<unsigned int>   leftEdgeVertices;
        std::vector<unsigned int>   leftNearVertices;
        std::vector<unsigned int>   rightEdgeVertices;
        std::vector<unsigned int>   rightNearVertices;
        unsigned int    curLeftQuadIndex;
        unsigned int    curRightQuadIndex;
        unsigned int    lastLeftQuadIndex;
        unsigned int    lastRightQuadIndex;
        unsigned int    totalRoadVertices = (bSteps + 1) * 2;
        for (int step = 0; step <= bSteps; ++step)
        {
            float   t = step * (1.0f / bSteps);
            float   roadHeight = BezierCurve(bHeightPoints, t)[1];
            bPoint = BezierCurve(bCtlPoints, t);

			roadVtxPositions->emplace_back(bPoint[0], roadHeight, bPoint[1]);    // on left edge of the road
			roadVtxPositions->emplace_back(bPoint[0] + roadWidth, roadHeight, bPoint[1]);    // on right edge of the road

			curLeftQuadIndex = roadVtxPositions->size() - 2;
			curRightQuadIndex = roadVtxPositions->size() - 1;

			curLeftSeg = glm::uvec2(segments[0] * roadVtxPositions->at(curLeftQuadIndex).x / width, segments[1] * (-roadVtxPositions->at(curLeftQuadIndex).z) / length);
			curRightSeg = glm::uvec2(segments[0] * roadVtxPositions->at(curRightQuadIndex).x / width, segments[1] * (-roadVtxPositions->at(curRightQuadIndex).z) / length);
            
            if (step == 0)
            {
                leftNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curLeftSeg[0], curLeftSeg[1], segments[1]));    // cur bottom left
                rightNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curRightSeg[0]+1, curRightSeg[1], segments[1]));  // cur bottom right
            }
            else
            {
                AddTriangleIndices(glm::uvec3(lastLeftQuadIndex, lastRightQuadIndex, curRightQuadIndex), roadTriangles);
                AddTriangleIndices(glm::uvec3(lastLeftQuadIndex, curRightQuadIndex, curLeftQuadIndex), roadTriangles);
                
                assert(   (curLeftSeg[0] == lastLeftSeg[0] && curLeftSeg[1] == lastLeftSeg[1])
                       || (curLeftSeg[0] == lastLeftSeg[0] && curLeftSeg[1] == lastLeftSeg[1] + 1)
                       || (curLeftSeg[0] == lastLeftSeg[0] + 1 && curLeftSeg[1] == lastLeftSeg[1])
                       || (curLeftSeg[0] == lastLeftSeg[0] + 1 && curLeftSeg[1] == lastLeftSeg[1] + 1)
                       || (curLeftSeg[0] == lastLeftSeg[0] - 1 && curLeftSeg[1] == lastLeftSeg[1])
                       || (curLeftSeg[0] == lastLeftSeg[0] - 1 && curLeftSeg[1] == lastLeftSeg[1] + 1)
                       );
                
                if (curLeftSeg[0] == lastLeftSeg[0] && curLeftSeg[1] == lastLeftSeg[1])
                {
                    leftEdgeVertices.push_back(curLeftQuadIndex);
                }
                else
                {
                    // Create mesh with edge vertices
                    leftEdgeVertices.push_back(curLeftQuadIndex);
                    // Get pivots
                    if (curLeftSeg[1] == lastLeftSeg[1]+1 && curLeftSeg[0] != lastLeftSeg[0]+1)
                    {
                        leftNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curLeftSeg[0], curLeftSeg[1], segments[1]));    // cur bottom left
                    }
                    else if (curLeftSeg[0] == lastLeftSeg[0]+1)
                    {
                        leftNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(lastLeftSeg[0], lastLeftSeg[1]+1, segments[1]));    // last top left
                        leftNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(lastLeftSeg[0]+1, lastLeftSeg[1]+1, segments[1]));    // last top right
                    }
                    else // if (curLeftSeg[0] == lastLeftSeg[0]-1)
                    {
                        leftNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curLeftSeg[0], curLeftSeg[1], segments[1]));    // cur bottom left
                    }
                    
                    AddTriangleStrip(leftNearVertices, leftEdgeVertices, terrainTriangles);
                    leftNearVertices.erase(leftNearVertices.begin(), leftNearVertices.begin()+leftNearVertices.size()-1);
                    leftEdgeVertices.erase(leftEdgeVertices.begin(), leftEdgeVertices.begin()+leftEdgeVertices.size()-1);
                }
                
                
                assert(   (curRightSeg[0] == lastRightSeg[0] && curRightSeg[1] == lastRightSeg[1])
                       || (curRightSeg[0] == lastRightSeg[0] && curRightSeg[1] == lastRightSeg[1] + 1)
                       || (curRightSeg[0] == lastRightSeg[0] + 1 && curRightSeg[1] == lastRightSeg[1])
                       || (curRightSeg[0] == lastRightSeg[0] + 1 && curRightSeg[1] == lastRightSeg[1] + 1)
                       || (curRightSeg[0] == lastRightSeg[0] - 1 && curRightSeg[1] == lastRightSeg[1])
                       || (curRightSeg[0] == lastRightSeg[0] - 1 && curRightSeg[1] == lastRightSeg[1] + 1)
                       );
                
                
                if (curRightSeg[0] == lastRightSeg[0] && curRightSeg[1] == lastRightSeg[1])
                {
                    rightEdgeVertices.push_back(curRightQuadIndex);
                }
                else
                {
                    // Create mesh with edge vertices
                    rightEdgeVertices.push_back(curRightQuadIndex);
                    // Get pivots
                    if (curRightSeg[1] == lastRightSeg[1]+1)
                    {
                        rightNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curRightSeg[0]+1, curRightSeg[1], segments[1]));    // cur bottom right
                    }
                    else if (curRightSeg[0] == lastRightSeg[0]+1)
                    {
                        rightNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(curRightSeg[0]+1, curRightSeg[1], segments[1]));    // cur bottom right
                    }
                    else // if (curRightSeg[0] == lastRightSeg[0]-1)
                    {
                        rightNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(lastRightSeg[0]+1, lastRightSeg[1]+1, segments[1]));    // last top right
                        rightNearVertices.push_back(totalRoadVertices + relativeVtxIdxAt(lastRightSeg[0], lastRightSeg[1]+1, segments[1]));    // last top left
                    }
                    
                    AddTriangleStrip(rightEdgeVertices, rightNearVertices, terrainTriangles);
                    rightNearVertices.erase(rightNearVertices.begin(), rightNearVertices.begin()+rightNearVertices.size()-1);
                    rightEdgeVertices.erase(rightEdgeVertices.begin(), rightEdgeVertices.begin()+rightEdgeVertices.size()-1);
                }
                
                // Bounding boxes
                if (curLeftSeg[1] != lastLeftSeg[1])
                {
                    assert(roadSegments.size() == lastLeftSeg[1]);
                    roadSegments.emplace_back(curMinLeftSeg[0], curMaxRightSeg[0]);
                    curMinLeftSeg = glm::uvec2(segments[0]+1, segments[1]+1);
                    curMaxRightSeg = glm::uvec2(0, 0);
                }
                
            }
            
            if (curMinLeftSeg[0] > curLeftSeg[0])
                curMinLeftSeg = glm::uvec2(curLeftSeg[0], 0);
            if (curMaxRightSeg[0] < curRightSeg[0])
                curMaxRightSeg = glm::uvec2(curRightSeg[0], 0);
            
            lastLeftSeg = curLeftSeg;
            lastRightSeg = curRightSeg;
            lastLeftQuadIndex = curLeftQuadIndex;
            lastRightQuadIndex = curRightQuadIndex;
        }
        
        // Terrain construction
        
        // Clone road vertices first for terrain
        terrainVtxPositions->insert(terrainVtxPositions->begin(), roadVtxPositions->begin(), roadVtxPositions->end());
        
        // Perlin noise
        noise::module::Perlin   perlin;
        
        unsigned int    aQuadIndex;
        unsigned int    bQuadIndex;
        unsigned int    cQuadIndex;
        unsigned int    dQuadIndex;
        
        // A 2-D grid with triangles having height determined by perlin noise
        
        float           xCoord;
        float           zCoord;
        float           yCoord;
        
        for (unsigned i = 0; i <= segments[0]; ++i)
        {
            for (unsigned k = 0; k <= segments[1]; ++k)
            {
                // Create the vertex at index (i, j)
                xCoord = gridStart[0] + i*segmentSize[0];
                zCoord = gridStart[1] - k*segmentSize[1];
                
                // Check if this quad intersects with triangles constructing the road
                bool    intersectsWithRoad = false;
                
                yCoord = fabs(perlin.GetValue(i*(1.0f/segments[0]), 0.0f, k*(1.0/segments[1])) * height);
                //yCoord = gridStart[1] + maxHeight*rand()/RAND_MAX;
                if (i > 0 && k > 0)
                {
                    intersectsWithRoad = roadSegments[k - 1][0] <= (i - 1) && (i - 1) <= roadSegments[k - 1][1];
                }
                
                //yCoord = 0.0f;
                
                terrainVtxPositions->emplace_back(xCoord,
                                              (i==0 || k==0 || i==segments[0] || k==segments[1]) ? 0.0f : yCoord,
                                              zCoord);
                
                // Add the triangles from the quad in CCW order
                if (i > 0 && k > 0 && !intersectsWithRoad)
                {
                    aQuadIndex = terrainVtxPositions->size() - segments[1] - 3;  // (i-1, j-1)
                    bQuadIndex = terrainVtxPositions->size() - 2;   // (i, j-1)
                    cQuadIndex = terrainVtxPositions->size() - 1;   // (i, j)
                    dQuadIndex = aQuadIndex + 1;    // (i-1, j)
                    
                    AddTriangleIndices(glm::uvec3(aQuadIndex, bQuadIndex, cQuadIndex), terrainTriangles);
                    AddTriangleIndices(glm::uvec3(aQuadIndex, cQuadIndex, dQuadIndex), terrainTriangles);
                }
            }
        }

		// Mountain construction

        std::vector<glm::uvec3> mountainLargeTriangles;
        
		// First grab vertices along z=-length
		for (unsigned i = 0; i <= segments[0]; ++i)
		{
			mountainVtxPositions->push_back(terrainVtxPositions->at(totalRoadVertices + relativeVtxIdxAt(i, segments[1], segments[1])));
		}

		float	curMaxMtnHeight = height;
		float	mtnSegmentLengthZ = mtnSegmentLength;
		for (unsigned k = 1; k <= 50; ++k)
		{
			curMaxMtnHeight *= 1.05;

			for (unsigned i = 0; i <= segments[0]; ++i)
			{
				// Create the vertex
				xCoord = gridStart[0] + i*segmentSize[0];
				zCoord = gridStart[1] - length - k * mtnSegmentLengthZ;
				//zCoord = gridStart[1] - 200 - k * mtnSegmentLengthZ;
				yCoord = fabs(perlin.GetValue(i*(1.0f / segments[0]), 0.0f, (1.0 + k * mtnSegmentLengthZ)) * curMaxMtnHeight);

				mountainVtxPositions->emplace_back(xCoord, yCoord, zCoord);

				// Add the triangles from the quad in CCW order
				if (i > 0 && k > 0)
				{
					aQuadIndex = mountainVtxPositions->size() - segments[0] - 3;  // (i-1, j-1)
					bQuadIndex = mountainVtxPositions->size() - segments[0] - 2;   // (i, j-1)
					cQuadIndex = mountainVtxPositions->size() - 1;   // (i, j)
					dQuadIndex = mountainVtxPositions->size() - 2;    // (i-1, j)

                    mountainLargeTriangles.emplace_back(aQuadIndex, bQuadIndex, cQuadIndex);
                    mountainLargeTriangles.emplace_back(aQuadIndex, cQuadIndex, dQuadIndex);
				}
			}
            // duplicate last row of vertices to avoid bad normals at sharp edges
            for (unsigned i = 0; i <= segments[0]; ++i)
            {
                mountainVtxPositions->push_back(mountainVtxPositions->at(mountainVtxPositions->size() - 1 - segments[0]));
            }
            
		}
        
        // Subdivide the triangles
        for (unsigned i = 0; i < mountainLargeTriangles.size(); ++i)
        {
            SimpleTriangleSubdivide(mountainLargeTriangles[i], *mountainVtxPositions, mountainTriangles, 0);
        }
        
        // Normals, UVs and rendering object for Road
#if 0
        for (unsigned i = 0; i < roadVtxPositions->size(); ++i)
            std::cout << "Road vertex at " << i << " : " << glm::to_string(roadVtxPositions->at(i)) << std::endl;
        
        for (unsigned i = 0; i < roadVtxIndices.size(); ++i)
            std::cout << "Road triangle at " << i << " : " << roadVtxIndices.at(i) << std::endl;
#endif
        
        std::unique_ptr<std::vector<glm::vec3> > roadVtxNormals = genAvgVtxNormals(*roadVtxPositions, roadTriangles);
        
        std::unique_ptr<std::vector<glm::vec2> > roadVtxUV = make_unique<std::vector<glm::vec2> >(roadVtxPositions->size());
        for (decltype(roadVtxUV->size()) i = 0; i < roadVtxUV->size(); ++i) {
            const glm::vec3 position = glm::vec3(roadVtxPositions->at(i));
            roadVtxUV->at(i) = glm::vec2(i % 2 == 0 ? 0.0f : 1.0f, glm::fract(-position.z/500.0));//(length - position.z) / length);
        }
        
        road = makeMeshObject(std::move(roadVtxPositions),
                              make_unique<std::vector<glm::uvec3> >(std::move(roadTriangles)),
                              std::move(roadVtxNormals),
                              std::move(roadVtxUV)
                            );
        
        // Normals, UVs and rendering object for Terrain
#if 0
        for (unsigned i = 0; i < terrainVtxPositions->size(); ++i)
            std::cout << "Terrain vertex at " << i << " : " << glm::to_string(terrainVtxPositions->at(i)) << std::endl;
        
        for (unsigned i = 0; i < terrainVtxIndices.size(); ++i)
            std::cout << "Terrain triangle at " << i << " : " << terrainVtxIndices.at(i) << std::endl;
#endif
        
        std::unique_ptr<std::vector<glm::vec3> > terrainVtxNormals = genAvgVtxNormals(*terrainVtxPositions, terrainTriangles);
        
        std::unique_ptr<std::vector<glm::vec2> > terrainVtxUV = make_unique<std::vector<glm::vec2> >(terrainVtxPositions->size());
        for (decltype(terrainVtxUV->size()) i = 0; i < terrainVtxUV->size(); ++i) {
            const glm::vec3 position = glm::vec3(terrainVtxPositions->at(i));
            terrainVtxUV->at(i) = glm::vec2(glm::fract(position.x/500), glm::fract(-position.z/500));
        }
        
        terrain = makeMeshObject(std::move(terrainVtxPositions),
                                 make_unique<std::vector<glm::uvec3> >(std::move(terrainTriangles)),
                                 std::move(terrainVtxNormals),
                                 std::move(terrainVtxUV)
                                                 );

		// Normals, UVs and rendering object for Mountain
#if 0
		for (unsigned i = 0; i < mountainVtxPositions->size(); ++i)
			std::cout << "Mountain vertex at " << i << " : " << glm::to_string(mountainVtxPositions->at(i)) << std::endl;

		for (unsigned i = 0; i < mountainVtxIndices.size(); ++i)
			std::cout << "Mountain triangle at " << i << " : " << mountainVtxIndices.at(i) << std::endl;
#endif

		std::unique_ptr<std::vector<glm::vec3> > mountainVtxNormals = genAreaWeightedAvgVtxNormals(*mountainVtxPositions, mountainTriangles);

		std::unique_ptr<std::vector<glm::vec2> > mountainVtxUV = make_unique<std::vector<glm::vec2> >(mountainVtxPositions->size());
		for (decltype(mountainVtxUV->size()) i = 0; i < mountainVtxUV->size(); ++i) {
			const glm::vec3 position = glm::vec3(mountainVtxPositions->at(i));
            mountainVtxUV->at(i) = glm::vec2(glm::fract(position.x/500), glm::fract(-position.z/500));
		}

		mountain = makeMeshObject(std::move(mountainVtxPositions),
                                  make_unique<std::vector<glm::uvec3> >(std::move(mountainTriangles)),
                                  std::move(mountainVtxNormals),
                                  std::move(mountainVtxUV)
                                  );

    }
}