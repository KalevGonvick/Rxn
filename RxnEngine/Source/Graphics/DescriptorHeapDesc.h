#pragma once

namespace Rxn::Graphics
{
    class DescriptorHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC
    {
    public:

        DescriptorHeapDesc(int numDescriptors);
        ~DescriptorHeapDesc();

    public:

        void SetMask(uint32 mask);
        void CreateRTVDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &rtvDescriptorHeap);
        void CreateSRVDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &srvDescriptorHeap);
        void CreateDSVDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &dsvDescriptorHeap);
        void CreateSamplerDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &samplerDescriptorHeap);

    };
}
