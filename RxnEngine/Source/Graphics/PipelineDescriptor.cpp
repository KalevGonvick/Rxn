#include "Rxn.h"
#include "PipelineDescriptor.h"

namespace Rxn::Graphics
{

#pragma region GraphicsPipelineDescriptor

    GraphcisPipelineDescriptor::GraphcisPipelineDescriptor() = default;
    GraphcisPipelineDescriptor::~GraphcisPipelineDescriptor() = default;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC &GraphcisPipelineDescriptor::BuildPipelineDesc(ID3D12RootSignature *rootSig)
    {
        pRootSignature = rootSig;
        SampleMask = UINT_MAX;
        RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        NumRenderTargets = 1;
        RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        SampleDesc.Count = 1;
        BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        DepthStencilState.DepthEnable = FALSE;
        DepthStencilState.StencilEnable = FALSE;

        return *this;
    }

    void GraphcisPipelineDescriptor::SetGraphicsShaderSet(const GraphicsShaderSet &graphicsShaderSet)
    {
        InputLayout = graphicsShaderSet.il;
        VS = graphicsShaderSet.vs;
        PS = graphicsShaderSet.ps;
        DS = graphicsShaderSet.ds;
        HS = graphicsShaderSet.hs;
        GS = graphicsShaderSet.gs;
    }

#pragma endregion // GraphicsPipelineDescriptor

#pragma region ComputePipelineDescriptor

    ComputePipelineDescriptor::ComputePipelineDescriptor() = default;
    ComputePipelineDescriptor::~ComputePipelineDescriptor() = default;

    D3D12_COMPUTE_PIPELINE_STATE_DESC &ComputePipelineDescriptor::BuildPipelineDesc(ID3D12RootSignature *rootSig)
    {
        pRootSignature = rootSig;
        return *this;
    }

    void ComputePipelineDescriptor::SetComputeShader(const D3D12_SHADER_BYTECODE &byteCode)
    {
        CS = byteCode;
    }

#pragma endregion // ComputePipelineDescriptor
}
