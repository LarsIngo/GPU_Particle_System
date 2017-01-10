#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include "ParticleCloud.h"
#include "Scene.h"

class ParticleCloudSystem
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        ParticleCloudSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~ParticleCloudSystem();

        // Bind pipeline.
        // sourceBuffer Buffer to read from.
        // numSRVs Number of srcBuffers.
        // targetBuffer Buffer to write to.
        // numUAVs Number of trgBuffers.
        void Bind(ID3D11ShaderResourceView** sourceBuffer, unsigned int numSRVs, ID3D11UnorderedAccessView** targetBuffer, unsigned int numUAVs);

        // Unbind pipeline.
        // numSRVs Number of srcBuffers.
        // numUAVs Number of trgBuffers.
        void Unbind(unsigned int numSRVs, unsigned int numUAVs);

        // Update particles.
        // scene Scene to update.
        // dt Delta time.
        void Update(Scene& scene, float dt);

        // MetaData.
        struct MetaData
        {
            float dt;
            unsigned int numClouds;
            float pad[2];
        } mMetaData;

    private:
        // Initialise buffers and compute shader.
        void Initialise();

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11ComputeShader* mParticleUpdateCS;
        ID3D11ShaderResourceView* mMetaDataBuffer;
};
