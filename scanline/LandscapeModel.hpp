//
//  LandscapeModel.hpp
//  cs148-opengl4
//
//  Created by Tanmoy on 10/1/15.
//
//

#ifndef LandscapeModel_hpp
#define LandscapeModel_hpp

#include <stdio.h>
#include "common/common.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"

namespace LandscapeModel {


    class LandscapeModel {
        
    private:
        
        std::shared_ptr<MeshObject>     road;
        std::shared_ptr<MeshObject>     terrain;
		std::shared_ptr<MeshObject>     mountain;
        
    public:
    
		LandscapeModel(float length, float width, float height, float roadWidth, float roadPivotWidthX, int steps, int roadStepsScale=100, float mtnSegmentLength=100);
    
        std::shared_ptr<MeshObject> GetTerrain()
        {
            return terrain;
        }
        std::shared_ptr<MeshObject> GetRoad()
        {
            return road;
        }
		std::shared_ptr<MeshObject> GetMountain()
		{
			return mountain;
		}
    };

}

#endif /* LandscapeModel_hpp */
