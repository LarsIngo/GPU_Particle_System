#include "ParticleSorter.h"

#include "DxAssert.h"
#include "DxHelp.h"

ParticleSorter::ParticleSorter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    // Create buffers.
    Initialise();
}

ParticleSorter::~ParticleSorter()
{
    mMetaDataBuffer->Release();
    mParticleSort01CS->Release();
    mParticleSort02CS->Release();
    mParticleSort03CS->Release();
}

void ParticleSorter::Bind(ID3D11ShaderResourceView* sourceBuffer, ID3D11UnorderedAccessView* targetBuffer, ID3D11ComputeShader* computeShader)
{
    mpDeviceContext->CSSetShader(computeShader, NULL, NULL);
    mpDeviceContext->CSSetShaderResources(0, 1, &sourceBuffer);
    mpDeviceContext->CSSetShaderResources(1, 1, &mMetaDataBuffer);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, &targetBuffer, NULL);
}

void ParticleSorter::Unbind()
{
    mpDeviceContext->CSSetShader(NULL, NULL, NULL);
    void *const p[1] = { NULL };
    mpDeviceContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)p, NULL);
}

void ParticleSorter::Sort(Scene& scene)
{
    unsigned int numParticles = scene.mMaxNumParticles;
    unsigned int dispDim = numParticles / 2;
    unsigned int numThreads = RoofPow2(dispDim);

    mMetaData.numThreads = numThreads;
    mMetaData.numParticles = numParticles;

    // TONIC INIT
    for (unsigned int step = 1; step <= numParticles / 2; step *= 2)
    {
        // Swap buffers.
        scene.mParticlesGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer(), mParticleSort01CS);

        mpDeviceContext->Dispatch(dispDim / 256 + 1, 1, 1);

        Unbind();
    }

    // TONIC SWAP
    for (unsigned int step = numParticles / 8; step >= 1; step /= 2)
    {
        // Swap buffers.
        scene.mParticlesGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer(), mParticleSort02CS);

        mpDeviceContext->Dispatch(dispDim / 256 + 1, 1, 1);

        Unbind();
    }

    // TONIC MERGE
    for (unsigned int step = numParticles / 2; step >= 1; step /= 2)
    {
        // Swap buffers.
        scene.mParticlesGPUSwapBuffer->Swap();

        // Update meta buffer.
        mMetaData.step = step;
        DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

        Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer(), mParticleSort03CS);

        mpDeviceContext->Dispatch(dispDim / 256 + 1, 1, 1);

        Unbind();
    }

    //for (unsigned int step = 1; step <= numPartices / 2; step *= 2)
    //{
    //    // Swap buffers.
    //    scene.mParticlesGPUSwapBuffer->Swap();

    //    // Update meta buffer.
    //    mMetaData.numParticles = numPartices;
    //    mMetaData.step = step;
    //    mMetaData.len = 4 * step;
    //    DxHelp::WriteStructuredBuffer<MetaData>(mpDeviceContext, &mMetaData, 1, mMetaDataBuffer);

    //    Bind(scene.mParticlesGPUSwapBuffer->GetSourceBuffer(), scene.mParticlesGPUSwapBuffer->GetTargetBuffer());

    //    unsigned int dim = numPartices / 2;
    //    mpDeviceContext->Dispatch(dim / 256 + 1, 1, 1);

    //    Unbind();
    //}
}

void ParticleSorter::Initialise()
{
    // Create meta buffer.
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<MetaData>(mpDevice, 1, &mMetaDataBuffer);

    // Create compute shader.
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr;
        
    hr = D3DCompileFromFile(L"Particles_Sort_01_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleSort01CS), S_OK);
    shaderBlob->Release();

    hr = D3DCompileFromFile(L"Particles_Sort_02_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleSort02CS), S_OK);
    shaderBlob->Release();

    hr = D3DCompileFromFile(L"Particles_Sort_03_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", 0, NULL, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        char* errorMsg = (char*)errorBlob->GetBufferPointer();
        OutputDebugStringA(errorMsg);
        shaderBlob->Release();
    }
    DxAssert(mpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &mParticleSort03CS), S_OK);
    shaderBlob->Release();
}

unsigned int ParticleSorter::RoofPow2(unsigned int v)
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
