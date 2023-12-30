#include "Rxn.h"
#include "SamplerDesc.h"

namespace Rxn::Graphics
{
    SamplerDesc::SamplerDesc()
    {
        Filter = D3D12_FILTER_ANISOTROPIC;
        AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        MipLODBias = 0.0f;
        MaxAnisotropy = 16;
        ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        BorderColor[0] = 1.0f;
        BorderColor[1] = 1.0f;
        BorderColor[2] = 1.0f;
        BorderColor[3] = 1.0f;
        MinLOD = 0.0f;
        MaxLOD = D3D12_FLOAT32_MAX;
    }

    SamplerDesc::~SamplerDesc() = default;

    void SamplerDesc::SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE AddressMode)
    {
        AddressU = AddressMode;
        AddressV = AddressMode;
        AddressW = AddressMode;
    }

    void SamplerDesc::SetBorderColour(FVector4D<float32> colour)
    {
        BorderColor[0] = colour.x;
        BorderColor[1] = colour.y;
        BorderColor[2] = colour.z;
        BorderColor[3] = colour.w;
    }

    void SamplerDesc::CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle) const
    {
        if (FAILED(handle.ptr))
        {
            return;
        }

        RenderContext::GetGraphicsDevice()->CreateSampler(this, handle);
    }
}
