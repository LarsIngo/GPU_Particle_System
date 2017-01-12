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
    int yAxis, xAxis;
    yAxis = xAxis = sqrt(mMaxNumParticleClouds);

    std::default_random_engine rng(time(0));
    std::uniform_int_distribution<int> dist(-1000, 1000);

    unsigned int index = 0;
    for (int y = 0; y < yAxis; ++y)
    {
        for (int x = 0; x < xAxis; ++x)
        {
            ParticleCloud particleCloud;
            particleCloud.mPosition = glm::vec3(x, 0.f, y) * space; // dist(rng) / 1000.f
            particleCloud.mRadius = 0.2f;
            particleCloud.mNumParticles = 8;
            particleCloud.mParticleStartID = index;
            index += particleCloud.mNumParticles;
            particleCloud.mVelocity = -glm::normalize(particleCloud.mPosition + glm::vec3(0.01f, 0.f, 0.01f)) * (float)((x + y) == 0);
            particleCloud.mColor = glm::vec3(0.f, 0.2f, 0.f);
            particleCloud.mSpawntime = 0.01f + abs(dist(rng) / 1000.f) * 0.05f;
            particleClouds.push_back(particleCloud);

            for (unsigned int i = 0; i < particleCloud.mNumParticles; ++i)
            {
                Particle particle;
                particle.mPosition = particleCloud.mPosition;
                particle.mPosition.y = 5.f * ((float)i / particleCloud.mNumParticles);
                particle.mScale = glm::vec2(0.2f, 0.2f) * 1.f;
                particle.mColor = glm::vec3(i % 3, i % 2, 1.f);
                particle.mVelocity = glm::vec3(particleCloud.mVelocity.x, 0.f, particleCloud.mVelocity.z);
                particle.mLifetime = -1.f; // particleCloud.mNumParticles - i;
                particles.push_back(particle);
            }
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
    if (particles.size() < mMaxNumParticles)
        for (unsigned int i = particles.size(); i < mMaxNumParticles; ++i)
            particles.push_back(Particle());
    //if (particleClouds.size() < mMaxNumParticleClouds)
    //    for (unsigned int i = 0; i < mMaxNumParticles - particleClouds.size(); ++i)
    //        particleClouds.push_back(ParticleCloud());
    assert(particles.size() == mMaxNumParticles);
    assert(particleClouds.size() == mMaxNumParticleClouds);
    mParticlesGPUSwapBuffer = new GPUSwapBuffer<Particle>(mpDevice, mpDeviceContext, mMaxNumParticles, particles.data());
    mParticleCloudsGPUSwapBuffer = new GPUSwapBuffer<ParticleCloud>(mpDevice, mpDeviceContext, mMaxNumParticleClouds, particleClouds.data());
}

Scene::~Scene()
{
    delete mParticlesGPUSwapBuffer;
    delete mParticleCloudsGPUSwapBuffer;
}
