#pragma once

#include "Camera.h"
#include "DynamicArray.hpp"
#include "Particle.h"

class Scene
{
    public:
        // Constructor.
        Scene();

        // Destructor.
        ~Scene();

        // Camera.
        Camera mCamera;

        // Particles.
        DynamicArray<Particle>* mParticles;
};
