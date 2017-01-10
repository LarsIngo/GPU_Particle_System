#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include "Camera.h"
#include "GPUSwapBuffer.h"
#include "Particle.h"
#include "ParticleCloud.h"

class Scene
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // maxNumParticles Max number of particles.
        // maxNumParticleClouds Max number of particle clouds.
        Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles, unsigned int maxNumParticleClouds);

        // Destructor.
        ~Scene();

        // Camera.
        Camera mCamera;

        // Max number paricles.
        unsigned int mMaxNumParticles;

        // Max number paricle clouds.
        unsigned int mMaxNumParticleClouds;

        // Partilce cloud GPU swap buffer.
        GPUSwapBuffer<ParticleCloud>* mParticleCloudsGPUSwapBuffer;

        // Particles GPU swap buffer.
        GPUSwapBuffer<Particle>* mParticlesGPUSwapBuffer;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
