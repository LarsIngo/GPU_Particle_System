#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.h>

#include "DxHelp.h"

template <typename T>
// Double-buffered swap buffer. Allocate two pars of read and write buffer on the GPU.
class GPUSwapBuffer 
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // numOfElements Number of elements of type elements T.
        // initData Init data.
        GPUSwapBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int numOfElements, T* initData = nullptr);

        // Destructor.
        ~GPUSwapBuffer();

        // Swap read/write buffers.
        void Swap();

        // Get source buffer (GPU read buffer).
        ID3D11ShaderResourceView* GetSourceBuffer();

        // Get target buffer (GPU write buffer).
        ID3D11UnorderedAccessView* GetTargetBuffer();

        // Update vertex buffer (copy data from target buffer to vertex buffer).
        void UpdateVertexBuffer();

        // Get vertex buffer.
        ID3D11Buffer* GetVertexBuffer();

    private:
        bool mState;
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11Buffer* mBuffers[2];
        ID3D11ShaderResourceView* mSourceBuffers[2];
        ID3D11UnorderedAccessView* mTargetBuffers[2];
        ID3D11Buffer* mVertexBuffer;
};

template <typename T>
inline GPUSwapBuffer<T>::GPUSwapBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int numOfElements, T* initData)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mState = 0;

    // Create source and target buffers.
    {
        D3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
        bDesc.ByteWidth = sizeof(T) * numOfElements;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bDesc.CPUAccessFlags = 0;
        bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bDesc.StructureByteStride = sizeof(T);
        if (initData == nullptr)
        {
            DxAssert(pDevice->CreateBuffer(&bDesc, NULL, &mBuffers[0]), S_OK);
            DxAssert(pDevice->CreateBuffer(&bDesc, NULL, &mBuffers[1]), S_OK);
        }
        else
        {
            D3D11_SUBRESOURCE_DATA sData;
            ZeroMemory(&sData, sizeof(D3D11_SUBRESOURCE_DATA));
            sData.pSysMem = initData;
            DxAssert(pDevice->CreateBuffer(&bDesc, &sData, &mBuffers[0]), S_OK);
            DxAssert(pDevice->CreateBuffer(&bDesc, &sData, &mBuffers[1]), S_OK);
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc;
        ZeroMemory(&srcDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srcDesc.Format = DXGI_FORMAT_UNKNOWN;
        srcDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srcDesc.Buffer.FirstElement = 0;
        srcDesc.Buffer.ElementOffset = 0;
        srcDesc.Buffer.NumElements = numOfElements;
        DxAssert(pDevice->CreateShaderResourceView(mBuffers[0], &srcDesc, &mSourceBuffers[0]), S_OK);
        DxAssert(pDevice->CreateShaderResourceView(mBuffers[1], &srcDesc, &mSourceBuffers[1]), S_OK);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = numOfElements;
        uavDesc.Buffer.Flags = 0;
        DxAssert(pDevice->CreateUnorderedAccessView(mBuffers[0], &uavDesc, &mTargetBuffers[0]), S_OK);
        DxAssert(pDevice->CreateUnorderedAccessView(mBuffers[1], &uavDesc, &mTargetBuffers[1]), S_OK);
    }
    
    // Create vertex buffer.
    {
        DxHelp::CreateVertexBuffer<Particle>(mpDevice, numOfElements, &mVertexBuffer);
    }
 }

template <typename T>
inline GPUSwapBuffer<T>::~GPUSwapBuffer()
{
    mSourceBuffers[0]->Release();
    mSourceBuffers[1]->Release();
    mTargetBuffers[0]->Release();
    mTargetBuffers[1]->Release();
    mBuffers[0]->Release();
    mBuffers[1]->Release();
    mVertexBuffer->Release();
}

template <typename T>
inline void GPUSwapBuffer<T>::Swap()
{
    mState = !mState;
}

template <typename T>
inline ID3D11ShaderResourceView* GPUSwapBuffer<T>::GetSourceBuffer()
{
    return mSourceBuffers[!mState];
}

template <typename T>
inline ID3D11UnorderedAccessView* GPUSwapBuffer<T>::GetTargetBuffer()
{
    return mTargetBuffers[mState];
}

template <typename T>
inline void GPUSwapBuffer<T>::UpdateVertexBuffer()
{
    mpDeviceContext->CopyResource(mVertexBuffer, mBuffers[mState]);
}

template <typename T>
inline ID3D11Buffer* GPUSwapBuffer<T>::GetVertexBuffer()
{
    return mVertexBuffer;
}
