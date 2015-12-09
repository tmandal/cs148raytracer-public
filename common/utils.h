//
//  utils.h
//  cs148-raytracer
//
//  Created by Tanmoy on 12/8/15.
//
//

#ifndef utils_h
#define utils_h


// Utility functions
inline float RandFloat01()
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

inline float glm_max_component(glm::vec3 vector)
{
    return (vector.x > vector.y)
    ? (vector.x > vector.z ? vector.x : vector.z)
    : (vector.y > vector.z ? vector.y : vector.z);
}

inline glm::vec3 rand_point(const Box& box)
{
    float   xShift = RandFloat01();
    float   yShift = RandFloat01();
    float   zShift = RandFloat01();
    
    return (box.minVertex + glm::vec3(xShift, yShift, zShift) * (box.maxVertex - box.minVertex));
}

#endif /* utils_h */
