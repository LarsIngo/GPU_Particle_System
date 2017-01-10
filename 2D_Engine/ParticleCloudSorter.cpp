#include "ParticleCloudSorter.h"

#include "DxAssert.h"
#include "DxHelp.h"

ParticleCloudSorter::ParticleCloudSorter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    // Create buffers.
    Initialise();
}

ParticleCloudSorter::~ParticleCloudSorter()
{
    mMetaDataBuffer->Release();
    mParticleCloudSort01CS->Release();
    mParticleCloudSort02CS->Release();
    mParticleCloudSort03CS->Release();
}

void ParticleCloudSorter::Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer, ID3D11ComputeShader* computeShader)
{
    mpDeviceContext->CSSetShader(computeShader, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, 1, &sourceBuffer);
    mpDeviceContext->CSSetShaderResources(1, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &targetBuffer, NULL);
}

void ParticleCloudSorter::Unbind()
{
    mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    void *const p[1] = { NULL };
    mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleCloudSorter::Sort(Scene& scene)
{
    unsigned int numClouds = scene.mMaxNumParticleClouds;

    unsigned int numThreads = RoofPow2(numClouds) / 2;

    mMetaData.numClouds = numClouds;
    mMetaData.numThreads = numThreads;
    mMetaData.init = true;

    // TONIC INIT
    for (unsigned int step = 1; step <= numThreads / 4; step *= 2)
    {
        // Swap buffers.
        scene.mParticleCloudsGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticleCloudsGPUSwapBuffer->GetSourceBuffer(), scene.mParticleCloudsGPUSwapBuffer->GetTargetBuffer(), mParticleCloudSort01CS);

        mpDeviceContext->Dispatch(numThreads / 256 + 1, 1, 1);

        Unbind();

        // Update init.
        mMetaData.init = false;
    }

    // TONIC SWAP
    for (unsigned int step = numThreads / 2; step >= 1; step /= 2)
    {
        // Swap buffers.
        scene.mParticleCloudsGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticleCloudsGPUSwapBuffer->GetSourceBuffer(), scene.mParticleCloudsGPUSwapBuffer->GetTargetBuffer(), mParticleCloudSort02CS);

        mpDeviceContext->Dispatch(numThreads / 256 + 1, 1, 1);

        Unbind();
    }

    // TONIC MERGE
    for (unsigned int step = numThreads; step >= 1; step /= 2)
    {
        // Swap buffers.
        scene.mParticleCloudsGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticleCloudsGPUSwapBuffer->GetSourceBuffer(), scene.mParticleCloudsGPUSwapBuffer->GetTargetBuffer(), mParticleCloudSort03CS);

        mpDeviceContext->Dispatch(numThreads / 256 + 1, 1, 1);

        Unbind();
    }
}

void ParticleCloudSorter::Initialise()
{
    // Create meta buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    // Create compute shader.
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr;
        
    hr = D3DCompileFromFile(L"ParticleClouds_Sort_01_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleCloudSort01CS), S_OK);
    shaderBlob->Release();

    hr = D3DCompileFromFile(L"ParticleClouds_Sort_02_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleCloudSort02CS), S_OK);
    shaderBlob->Release();

    hr = D3DCompileFromFile(L"ParticleClouds_Sort_03_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleCloudSort03CS), S_OK);
    shaderBlob->Release();
}

unsigned int ParticleCloudSorter::RoofPow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
