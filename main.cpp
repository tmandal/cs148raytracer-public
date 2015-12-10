#include "common/RayTracer.h"
#include <cstdlib>

//#define ASSIGNMENT 8
#if ASSIGNMENT == 5
#define APPLICATION Assignment5
#include "assignment5/Assignment5.h"
#elif ASSIGNMENT == 6
#define APPLICATION Assignment6
#include "assignment6/Assignment6.h"
#elif ASSIGNMENT == 7
#define APPLICATION Assignment7
#include "assignment7/Assignment7.h"
#elif ASSIGNMENT == 8
#define APPLICATION Assignment8
#include "assignment8/Assignment8.h"
#else
#if 0
#define APPLICATION Scanline
#include "scanline/Scanline.h"
#else
#define APPLICATION RtImage
#include "rtimage/RtImage.h"
#endif
#endif

#ifdef _WIN32
#define WAIT_ON_EXIT 1
#else
#define WAIT_ON_EXIT 0
#endif

int main(int argc, char** argv)  
{
    unsigned int seed = static_cast<unsigned int>(time(NULL));
    std::string outputFilename = "output.png";
    glm::vec2   imageResolution = glm::vec2(640.f, 480.f);
    glm::uvec2  imageGridIndex = glm::uvec2(0, 0);
    glm::uvec2  imageGridSize = glm::uvec2(480, 640);
    if (argc > 8)
    {
        seed = strtol(argv[1], NULL, 0);
        imageResolution.x = strtol(argv[2], NULL, 0);
        imageResolution.y = strtol(argv[3], NULL, 0);
        imageGridIndex.x  = strtol(argv[4], NULL, 0);
        imageGridIndex.y  = strtol(argv[5], NULL, 0);
        imageGridSize.x   = strtol(argv[6], NULL, 0);
        imageGridSize.y   = strtol(argv[7], NULL, 0);
        outputFilename = argv[8];
    }

    srand(seed);
    std::cout << "Using seed : " << seed << std::endl;

    std::unique_ptr<APPLICATION> currentApplication = make_unique<APPLICATION>();
    currentApplication->SetOutputFilename(outputFilename);
    currentApplication->SetImageOutputResolution(imageResolution);
    currentApplication->SetImageGridIndex(imageGridIndex);
    currentApplication->SetImageGridSize(imageGridSize);
    RayTracer rayTracer(std::move(currentApplication));

    DIAGNOSTICS_TIMER(timer, "Ray Tracer");
    rayTracer.Run();
    DIAGNOSTICS_END_TIMER(timer);

    DIAGNOSTICS_PRINT();

#if defined(_WIN32) && WAIT_ON_EXIT
    int exit = 0;
    std::cin >> exit;
#endif

    return 0;
}
