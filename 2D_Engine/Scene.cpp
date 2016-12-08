#include "Scene.h"

#include "DxHelp.h"

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mMaxNumParticles = maxNumParticles;

    // Populate particles array.
    unsigned int numParticles = maxNumParticles;
    DynamicArray<Particle> mParticles(numParticles);
    //float r = 5.f;
    //float s = r / std::cbrt(numParticles) * 2; // Works-ish.
    //for (float z = -r; z <= r; z += s) 
    //{
    //    for (float y = -r; y <= r; y += s)
    //    {
    //        for (float x = -r; x <= r; x += s)
    //        {
    //            if (mParticles.Size() < numParticles) {
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

    for (int x = 0; x < numParticles; ++x)
    {
        Particle particle = Particle();
        particle.mPosition = glm::vec3(x, 0, 0);
        particle.mScale = glm::vec2(0.45f, 0.45f);
        particle.mVelocity = glm::vec3(0.f, 0.f, 0.f);
        particle.mColor = glm::vec3((float)x / numParticles, 0.2f, 0.f);
        mParticles.Push(particle);
    }

    // Create buffer and init particle data.
    mParticlesGPUSwapBuffer = new GPUSwapBuffer<Particle>(mpDevice, mpDeviceContext, mMaxNumParticles, mParticles.GetArrPointer());
    mParticles.Delete();
}

Scene::~Scene()
{
    delete mParticlesGPUSwapBuffer;
}
