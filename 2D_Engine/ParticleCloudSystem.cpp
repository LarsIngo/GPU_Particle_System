#include "ParticleCloudSystem.h"

#include "DxHelp.h"

ParticleCloudSystem::ParticleCloudSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    Initialise();
}

ParticleCloudSystem::~ParticleCloudSystem()
{
    mParticleUpdateCS->Release();
    mMetaDataBuffer->Release();
}

void ParticleCloudSystem::Bind(ID3D11ShaderResourceView** sourceBuffer, unsigned int numSRVs, ID3D11UnorderedAccessView** targetBuffer, unsigned int numUAVs)
{
    mpDeviceContext->CSSetShader(mParticleUpdateCS, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, numSRVs, sourceBuffer);
    mpDeviceContext->CSSetShaderResources(numSRVs, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, numUAVs, targetBuffer, NULL);
}

void ParticleCloudSystem::Unbind(unsigned int numSRVs, unsigned int numUAVs)
{
    mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    void *const p[1] = { NULL };
    for (unsigned int i = 0; i < numSRVs + 1; ++i)
        mpDeviceContext->CSSetShaderResources(i, 1, (ID3D11ShaderResourceView**)p);
    for (unsigned int i = 0; i < numUAVs; ++i)
        mpDeviceContext->CSSetUnorderedAccessViews(i, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleCloudSystem::Update(Scene& scene, float dt)
{
    // Swap buffers.
    scene.mParticleCloudsGPUSwapBuffer->Swap();
    scene.mParticlesGPUSwapBuffer->Swap();

    unsigned int numClouds = scene.mMaxNumParticleClouds;

    // Update meta buffer.
    mMetaData.dt = dt;
    mMetaData.numClouds = numClouds;
    DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

    ID3D11ShaderResourceView* SRVs[2] = { scene.mParticleCloudsGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetSourceBuffer() };
    ID3D11UnorderedAccessView* UAVs[2] = { scene.mParticleCloudsGPUSwapBuffer->GetTargetBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer() };
    Bind(SRVs, 2, UAVs, 2);

    mpDeviceContext->Dispatch(numClouds / 256 + 1, 1, 1);

    Unbind(2, 2);
}

void ParticleCloudSystem::Initialise()
{
    // Create meta buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    // Create compute shader.
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"ParticleClouds_Update_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleUpdateCS), S_OK);
    shaderBlob->Release();
}
