#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include "Camera.h"
#include "DynamicArray.hpp"
#include "GPUSwapBuffer.h"
#include "Particle.h"

class Scene
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // maxNumParticles Max number of particles.
        Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int maxNumParticles);

        // Destructor.
        ~Scene();

        // Camera.
        Camera mCamera;

        // Max number paricles.
        unsigned int mMaxNumParticles;

        // Active number particles.
        unsigned int mActiveNumParticles;

        // Particles GPU swap buffer.
        GPUSwapBuffer<Particle>* mParticlesGPUSwapBuffer;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
