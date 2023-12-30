#include "Rxn.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    DescriptorHeapDesc::DescriptorHeapDesc(int32 numDescriptors)
    {
        NumDescriptors = numDescriptors;
        NodeMask = 0;
    }

    DescriptorHeapDesc::~DescriptorHeapDesc() = default;

    void DescriptorHeapDesc::SetMask(uint32 mask)
    {
        NodeMask = mask;
    }

    void DescriptorHeapDesc::CreateRTVDescriptorHeap(ID3D12DescriptorHeap **rtvDescriptorHeap)
    {
        Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(this, IID_PPV_ARGS(rtvDescriptorHeap));
    }

    void DescriptorHeapDesc::CreateSRVDescriptorHeap(ID3D12DescriptorHeap **srvDescriptorHeap)
    {
        Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(this, IID_PPV_ARGS(srvDescriptorHeap));
    }

    void DescriptorHeapDesc::CreateDSVDescriptorHeap(ID3D12DescriptorHeap **dsvDescriptorHeap)
    {
        Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(this, IID_PPV_ARGS(dsvDescriptorHeap));
    }

    void DescriptorHeapDesc::CreateSamplerDescriptorHeap(ID3D12DescriptorHeap **samplerDescriptorHeap)
    {
        Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(this, IID_PPV_ARGS(samplerDescriptorHeap));
    }


}
