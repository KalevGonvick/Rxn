#include "Rxn.h"
#include "Renderable.h"

namespace Rxn::Graphics::Basic
{
    Renderable::Renderable() = default;
    Renderable::~Renderable() = default;

    HRESULT Renderable::CreateCommittedBufferDestinationResource(ID3D12Device8 *device, ID3D12Resource **buffer, uint32 size) const
    {
        HRESULT result;

        const CD3DX12_RESOURCE_DESC resourceDescriptorBufferSize = CD3DX12_RESOURCE_DESC::Buffer(size);
        const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        result = device->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptorBufferSize, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(buffer));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for default heap.");
            return result;
        }

        return S_OK;
    }

    HRESULT Renderable::CreateCommittedUploadBufferResource(ID3D12Device8 *device, ID3D12Resource **buffer, uint32 size) const
    {
        HRESULT result;

        const CD3DX12_RESOURCE_DESC resourceDescriptorBufferSize = CD3DX12_RESOURCE_DESC::Buffer(size);
        const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptorBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(buffer));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for upload heap.");
            return result;
        }

        return S_OK;
    }

    uint8 *Renderable::MapAndGetHeapLocationFromBuffer(ID3D12Resource *buffer)
    {
        uint8 *mem = nullptr;
        CD3DX12_RANGE readRange(0, 0);
        buffer->Map(0, &readRange, std::bit_cast<void **>(&mem));
        return mem;
    }

    void Renderable::AddCopyRegionTransition(ID3D12GraphicsCommandList6 *cmdList, ID3D12Resource **buffer, uint32 size, ID3D12Resource **uploadBuffer) const
    {
        cmdList->CopyBufferRegion(*buffer, 0, *uploadBuffer, 0, size);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(*buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        cmdList->ResourceBarrier(1, &transition);
    }
}

