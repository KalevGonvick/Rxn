#pragma once

namespace Rxn::Graphics
{
    class DescriptorHeapDesc : D3D12_DESCRIPTOR_HEAP_DESC
    {
    public:

        DescriptorHeapDesc(int numDescriptors);
        ~DescriptorHeapDesc();

    public:

        // TODO these are null
        void CreateRTVDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &rtvDescriptorHeap);
        void CreateSRVDescriptorHeap(ComPointer<ID3D12DescriptorHeap> &srvDescriptorHeap);
    };
}
