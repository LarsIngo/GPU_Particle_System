#pragma once

#include <glm/glm.hpp>

// A particle.
class Particle 
{
    public:
        // Constructor.
        Particle();

        // Destructor.
        ~Particle();

        // Position.
        glm::vec3 mPosition;

        // Scale.
        glm::vec2 mScale;

        // Color.
        glm::vec3 mColor;

        // Velocity.
        glm::vec3 mVelocity;

        // Lifetime. If lifetim is less then zero, particle is inactive and not rendered.
        float mLifetime;
};
