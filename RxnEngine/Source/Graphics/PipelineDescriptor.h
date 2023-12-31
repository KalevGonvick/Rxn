#pragma once

namespace Rxn::Graphics
{
    struct GraphicsShaderSet
    {
        D3D12_INPUT_LAYOUT_DESC     il;
        D3D12_SHADER_BYTECODE       vs;
        D3D12_SHADER_BYTECODE       ps;
        D3D12_SHADER_BYTECODE       ds;
        D3D12_SHADER_BYTECODE       hs;
        D3D12_SHADER_BYTECODE       gs;
    };

    template<typename CT>
    class RXN_ENGINE_API PipelineDescriptor : public CT
    {

    public:

        PipelineDescriptor() = default;
        ~PipelineDescriptor() = default;

    public:

        virtual CT &BuildPipelineDesc(ID3D12RootSignature *rootSig) = 0;
    };

    class RXN_ENGINE_API GraphcisPipelineDescriptor : public PipelineDescriptor<D3D12_GRAPHICS_PIPELINE_STATE_DESC>
    {
    public:

        GraphcisPipelineDescriptor();
        ~GraphcisPipelineDescriptor();

    public:

        D3D12_GRAPHICS_PIPELINE_STATE_DESC &BuildPipelineDesc(ID3D12RootSignature *rootSig) override;
        void SetGraphicsShaderSet(const GraphicsShaderSet &graphicsShaderSet);

    };

    class RXN_ENGINE_API ComputePipelineDescriptor : public PipelineDescriptor<D3D12_COMPUTE_PIPELINE_STATE_DESC>
    {
    public:

        ComputePipelineDescriptor();
        ~ComputePipelineDescriptor();

    public:

        D3D12_COMPUTE_PIPELINE_STATE_DESC &BuildPipelineDesc(ID3D12RootSignature *rootSig) override;
        void SetComputeShader(const D3D12_SHADER_BYTECODE &byteCode);

    };
}