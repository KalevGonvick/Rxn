#pragma once

namespace Rxn::Graphics
{
    class DescriptorHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC
    {
    public:

        explicit DescriptorHeapDesc(int32 numDescriptors);
        ~DescriptorHeapDesc();

    public:

        void SetMask(uint32 mask);
        void CreateRTVDescriptorHeap(ID3D12DescriptorHeap **rtvDescriptorHeap);
        void CreateSRVDescriptorHeap(ID3D12DescriptorHeap **srvDescriptorHeap);
        void CreateDSVDescriptorHeap(ID3D12DescriptorHeap **dsvDescriptorHeap);
        void CreateSamplerDescriptorHeap(ID3D12DescriptorHeap **samplerDescriptorHeap);

    };
}
