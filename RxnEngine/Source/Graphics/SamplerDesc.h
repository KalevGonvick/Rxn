#pragma once

namespace Rxn::Graphics
{
    class SamplerDesc : public D3D12_SAMPLER_DESC
    {
    public:

        SamplerDesc();
        ~SamplerDesc();

    public:

        void SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE AddressMode);
        void SetBorderColour(Core::Math::FVector4D<float> vector);

        //D3D12_CPU_DESCRIPTOR_HANDLE CreateDescriptor();
        void CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle);
    };
}
