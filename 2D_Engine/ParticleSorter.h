#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include <glm/glm.hpp>

#include "Particle.h"
#include "Camera.h"
#include "Scene.h"

class ParticleSorter
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        ParticleSorter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~ParticleSorter();

        // Sort particles.
        // scene Scene to sort.
        void Sort(Scene& scene);

        // MetaData.
        struct MetaData
        {
            unsigned int step;
            unsigned int numParticles;
            unsigned int numThreads;
            bool init;
        } mMetaData;

    private:
        // Initialise buffers and compute shader.
        void Initialise();

        // Bind pipeline.
        // sourceBuffer Buffer to read from.
        // targetBuffer Buffer to write to.
        // computeShader Shader to use.
        void Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer, ID3D11ComputeShader* computeShader);

        // Unbind pipeline.
        void Unbind();

        // Returns closest factor to pow 2.
        unsigned int RoofPow2(unsigned int v);

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11ComputeShader* mParticleSort01CS;
        ID3D11ComputeShader* mParticleSort02CS;
        ID3D11ComputeShader* mParticleSort03CS;
        ID3D11ShaderResourceView* mMetaDataBuffer;
};
