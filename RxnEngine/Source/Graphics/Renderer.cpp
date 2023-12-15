#include "Rxn.h"
#include "Renderer.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int32 width, int32 height)
        : m_Display(width, height)
    {
        
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
    }

    Renderer::~Renderer() = default;

    void Renderer::CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
    {


        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        std::vector<uint8> texture = GenerateTextureData();
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = static_cast<int64>(TextureWidth) * TextureBytesPerPixel;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        m_Scene.AddTexture(textureUploadHeap, textureDesc, m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get(), textureData);

    }

    // Generate a simple black and white checkerboard texture.
    std::vector<uint8> Renderer::GenerateTextureData() const
    {
        const uint32 rowPitch = TextureWidth * TextureBytesPerPixel;
        const uint32 cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
        const uint32 cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const uint32 textureSize = rowPitch * TextureHeight;

        std::vector<uint8> data(textureSize);
        uint8 *pData = &data[0];

        for (uint32 n = 0; n < textureSize; n += TextureBytesPerPixel)
        {
            uint32 x = n % rowPitch;
            uint32 y = n / rowPitch;
            uint32 i = x / cellPitch;
            uint32 j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;        // R
                pData[n + 1] = 0x00;    // G
                pData[n + 2] = 0x00;    // B
                pData[n + 3] = 0xff;    // A
            }
            else
            {
                pData[n] = 0xff;        // R
                pData[n + 1] = 0xff;    // G
                pData[n + 2] = 0xff;    // B
                pData[n + 3] = 0xff;    // A
            }
        }

        return data;
    }

    Scene &Renderer::GetScene()
    {
        return m_Scene;
    }

    GPU::Fence &Renderer::GetFence()
    {
        return m_Fence;
    }

    Display &Renderer::GetDisplay()
    {
        return m_Display;
    }

    Manager::CommandQueueManager &Renderer::GetCommandQueueManager()
    {
        return m_CommandQueueManager;
    }

    Manager::CommandListManager &Renderer::GetCommandListManager()
    {
        return m_CommandListManager;
    }

    ComPointer<ID3D12CommandAllocator> &Renderer::GetCommandAllocator(const uint32 frameIndex)
    {
        return m_CommandAllocators[frameIndex];
    }

    Pooled::CommandAllocatorPool &Renderer::GetCommandAllocatorPool()
    {
        return m_AllocatorPool;
    }

    Mapped::PipelineLibrary &Renderer::GetPipelineLibrary()
    {
        return m_PipelineLibrary;
    }

    const uint32 &Renderer::GetDrawIndex() const
    {
        return m_DrawIndex;
    }

    void Renderer::IncrementDrawIndex()
    {
        m_DrawIndex++;
    }

    void Renderer::ResetDrawIndex()
    {
        m_DrawIndex = 0;
    }

    void Renderer::CreateAllocatorPool()
    {
        m_AllocatorPool.Create(RenderContext::GetGraphicsDevice());
    }

    void Renderer::InitRendererDisplay(Renderer &renderer, const String &cmdQueueHashKey)
    {
        renderer.m_Display.GetSwapChain().SetTearingSupport(renderer.m_HasTearingSupport);
        renderer.m_Display.GetSwapChain().CreateSwapChain(renderer.m_CommandQueueManager.GetCommandQueue(cmdQueueHashKey));
        renderer.m_Display.TurnOverSwapChainBuffer();

    }

    void Renderer::InitScene()
    {
        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[RootParametersCount]{};
        ranges[RootParameterSRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};
        rootParameters[RootParameterUberShaderCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterCB].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterSRV].InitAsDescriptorTable(1, &ranges[RootParameterSRV], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        m_Scene.InitHeaps();
        m_Scene.SetSerializedRootSignature(rootSignatureDesc);
        m_Scene.InitSceneRenderTargets(
            m_Display.GetFrameIndex(),
            m_Display.GetSwapChain()
        );
        m_Scene.GetDynamicConstantBuffer().Create(RenderContext::GetGraphicsDevice().Get());
    }

    void Renderer::InitCachedPipeline(Renderer &renderer, ComPointer<ID3D12Device> device)
    {
        renderer.GetPipelineLibrary().Build(device, renderer.GetScene().GetRootSignature());
    }

    void Renderer::InitCommandQueues(Renderer &renderer, const String &cmdQueueHashKey)
    {
        renderer.GetCommandQueueManager().CreateCommandQueue(cmdQueueHashKey);
    }

    void Renderer::InitCommandList(Renderer &renderer, const String &cmdQueueHashKey, const String &newCmdListHashKey)
    {
        auto primaryCommandQueue = renderer.GetCommandQueueManager().GetCommandQueue(cmdQueueHashKey);
        auto currentCommandAllocator = renderer.GetCommandAllocator(renderer.GetDisplay().GetFrameIndex());

        renderer.GetCommandListManager().CreateCommandList(
            newCmdListHashKey,
            currentCommandAllocator,
            true,
            D3D12_COMMAND_LIST_TYPE_DIRECT
        );

        //renderer.GetCommandListManager().CloseCommandList(newCmdListHashKey);
        renderer.GetCommandListManager().ExecuteCommandList(newCmdListHashKey, primaryCommandQueue);
        /*renderer.GetCommandListManager().CloseAndExecuteCommandList(
            newCmdListHashKey,
            primaryCommandQueue
        );*/
    }

    void Renderer::InitCommandAllocator(Renderer &renderer, uint32 bufferIndex)
    {
        renderer.GetCommandAllocator(bufferIndex) = renderer.GetCommandAllocatorPool().RequestAllocator(bufferIndex);
    }

    void Renderer::InitGpuFence(Renderer &renderer, const String &cmdQueueHashKey)
    {
        renderer.GetFence().CreateFence(renderer.GetDisplay().GetFrameIndex());
        const uint32 nextFrameIndex = renderer.GetDisplay().GetSwapChain().GetCurrentBackBufferIndex();

        renderer.GetFence().MoveFenceMarker(renderer.GetCommandQueueManager().GetCommandQueue(cmdQueueHashKey), renderer.GetDisplay().GetFrameIndex(), nextFrameIndex);
        renderer.GetDisplay().TurnOverSwapChainBuffer();
        renderer.GetFence().SignalFence(renderer.GetCommandQueueManager().GetCommandQueue(cmdQueueHashKey), renderer.GetDisplay().GetFrameIndex());
        renderer.GetFence().WaitInfinite(renderer.GetDisplay().GetFrameIndex());
        renderer.GetFence().IncrementFenceValue(renderer.GetDisplay().GetFrameIndex());
    }

   

}
