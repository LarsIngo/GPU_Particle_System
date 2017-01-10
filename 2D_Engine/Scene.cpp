#include "Scene.h"

#include "DxHelp.h"

#include <random>
#include <time.h>
#include <vector>

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles, unsigned int maxNumParticleClouds)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mMaxNumParticles = maxNumParticles;
    mMaxNumParticleClouds = maxNumParticleClouds;

    // Populate particles array.
    std::vector<Particle> particles;
    std::vector<ParticleCloud> particleClouds;

    float space = 0.5f;
    glm::vec3 origin = glm::vec3(0.f, 0.f, 0.f);
    int yAxis, xAxis;
    yAxis = xAxis = sqrt(mMaxNumParticles);

    std::default_random_engine rng(time(0));
    std::uniform_int_distribution<int> dist(-1000, 1000);

    unsigned int index = 0;
    for (int y = 0; y < yAxis; ++y)
    {
        for (int x = 0; x < xAxis; ++x)
        {
            Particle particle = Particle();
            particle.mPosition = origin + glm::vec3(x, 0.f, y) * space; // dist(rng) / 1000.f
            particle.mScale = glm::vec2(0.2f, 0.2f) * 1.f;
            particle.mColor = glm::vec3(0.f, 0.2f, 0.f);
            particle.mVelocity = -glm::normalize(particle.mPosition - origin) * 4.f; // glm::vec3(dist(rng)/1000.f, dist(rng) / 1000.f, dist(rng) / 1000.f); // glm::normalize(particle.mPosition); /
            //particle.mLifetime = -0.1f;
            particles.push_back(particle);

            ParticleCloud particleCloud;
            particleCloud.mPosition = particle.mPosition;
            particleCloud.mRadius = particle.mScale.x;
            particleCloud.mParticleStartID = index++;
            particleCloud.mNumParticles = 1;
            particleCloud.mVelocity = particle.mVelocity;
            particleClouds.push_back(particleCloud);
        }
    }

    //float r = 5.f;
    //float s = r / std::cbrt(mMaxNumParticles) * 2; // Works-ish.
    //for (float z = -r; z <= r; z += s) 
    //{
    //    for (float y = -r; y <= r; y += s)
    //    {
    //        for (float x = -r; x <= r; x += s)
    //        {
    //            if (mParticles.Size() < mMaxNumParticles) {
    //                Particle particle = Particle();
    //                particle.mPosition = glm::vec3(x + 0.01f * y + 0.01f * z, y, z);
    //                particle.mScale = glm::vec2(0.2f, 0.2f);
    //                particle.mVelocity = glm::vec3(0.f, 0.f, 0.f);
    //                particle.mColor = glm::abs(glm::normalize(particle.mPosition));
    //                mParticles.Push(particle);
    //            }
    //        }
    //    }
    //}

    //for (int x = 0; x < mMaxNumParticles; ++x)
    //{
    //    Particle particle = Particle();
    //    particle.mPosition = glm::vec3(x, 0, (float)x / mMaxNumParticles * 12);
    //    particle.mScale = glm::vec2(0.45f, 0.45f);
    //    particle.mVelocity = glm::vec3(-((float)x/ mMaxNumParticles * (float)x / mMaxNumParticles), 0.f, 0.f);
    //    particle.mColor = glm::vec3(0.f, 0.2f, 0.f);
    //    mParticles.Push(particle);
    //}

    //for (int x = 0; x < mMaxNumParticles; ++x)
    //{
    //    Particle particle = Particle();
    //    particle.mPosition = glm::vec3(x, (float)x / mMaxNumParticles, 0);
    //    particle.mScale = glm::vec2(0.45f, 0.45f);
    //    particle.mVelocity = glm::vec3(0.f, 0.f, 0.f);
    //    particle.mColor = glm::vec3((float)x / mMaxNumParticles, 0.2f, 0.f);
    //    mParticles.Push(particle);
    //}

    // Create buffer and init particle data.
    mParticlesGPUSwapBuffer = new GPUSwapBuffer<Particle>(mpDevice, mpDeviceContext, mMaxNumParticles, particles.data());
    mParticleCloudsGPUSwapBuffer = new GPUSwapBuffer<ParticleCloud>(mpDevice, mpDeviceContext, mMaxNumParticleClouds, particleClouds.data());
}

Scene::~Scene()
{
    delete mParticlesGPUSwapBuffer;
    delete mParticleCloudsGPUSwapBuffer;
}
