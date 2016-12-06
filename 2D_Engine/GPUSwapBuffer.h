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
        GPUSwapBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int numOfElements);

        // Destructor.
        ~GPUSwapBuffer();

        // Swap read/write buffers.
        void Swap();

        // Get source buffer (GPU read buffer).
        ID3D11ShaderResourceView* GetSourceBuffer();

        // Get target buffer (GPU write buffer).
        ID3D11UnorderedAccessView* GetTargetBuffer();

        // Write buffer.
        // data Data to write to GPU.
        // numOfElements Number of elements in data array.
        void Write(T* data, unsigned int numOfElements);

        // Read buffer.
        // Return pointer to data.
        T* Read();

    private:
        bool mState;
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
        ID3D11ShaderResourceView* mSourceBuffers[2];
        ID3D11UnorderedAccessView* mTargetBuffers[2];
};

template <typename T>
inline GPUSwapBuffer<T>::GPUSwapBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int numOfElements)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mState = 0;

    ID3D11Buffer* localBuffer[2];
    {
        D3D11_BUFFER_DESC bDesc;
        ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
        bDesc.ByteWidth = sizeof(T) * numOfElements;
        bDesc.Usage = D3D11_USAGE_DEFAULT;
        bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bDesc.StructureByteStride = sizeof(T);
        DxAssert(pDevice->CreateBuffer(&bDesc, NULL, &localBuffer[0]), S_OK);
        DxAssert(pDevice->CreateBuffer(&bDesc, NULL, &localBuffer[1]), S_OK);
    }
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc;
        ZeroMemory(&srcDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srcDesc.Format = DXGI_FORMAT_UNKNOWN;
        srcDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srcDesc.Buffer.FirstElement = 0;
        srcDesc.Buffer.ElementOffset = 0;
        srcDesc.Buffer.NumElements = numOfElements;
        DxAssert(pDevice->CreateShaderResourceView(localBuffer[0], &srcDesc, &mSourceBuffers[0]), S_OK);
        DxAssert(pDevice->CreateShaderResourceView(localBuffer[1], &srcDesc, &mSourceBuffers[1]), S_OK);
    }
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = numOfElements;
        uavDesc.Buffer.Flags = 0;
        DxAssert(pDevice->CreateUnorderedAccessView(localBuffer[0], &uavDesc, &mTargetBuffers[0]), S_OK);
        DxAssert(pDevice->CreateUnorderedAccessView(localBuffer[1], &uavDesc, &mTargetBuffers[1]), S_OK);
    }

    localBuffer[0]->Release();
    localBuffer[1]->Release();
}

template <typename T>
inline GPUSwapBuffer<T>::~GPUSwapBuffer()
{
    mSourceBuffers[0]->Release();
    mSourceBuffers[1]->Release();
    mTargetBuffers[0]->Release();
    mTargetBuffers[1]->Release();
}

template <typename T>
inline void GPUSwapBuffer<T>::Swap()
{
    mState = !mState;
}

template <typename T>
inline ID3D11ShaderResourceView* GPUSwapBuffer<T>::GetSourceBuffer()
{
    return mSourceBuffers[mState];
}

template <typename T>
inline ID3D11UnorderedAccessView* GPUSwapBuffer<T>::GetTargetBuffer()
{
    return mTargetBuffers[mState];
}

template <typename T>
inline void GPUSwapBuffer<T>::Write(T* data, unsigned int numOfElements)
{
    DxHelp::WriteStructuredBuffer<T>(mpDeviceContext, data, numOfElements, mSourceBuffers[mState]);
}

template <typename T>
inline T* GPUSwapBuffer<T>::Read()
{
    return DxHelp::ReadStructuredBuffer<T>(mpDeviceContext, mTargetBuffers[mState]);
}
