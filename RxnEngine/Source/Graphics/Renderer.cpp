#include "Rxn.h"
#include "Renderer.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int32 width, int32 height)
        : m_Display(width, height)
    {
        
        WCHAR assetsPath[512];
        Core::Strings::GetAssetsPath(assetsPath, _countof(assetsPath));
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

    void Renderer::InitializeRender()
    {
        GetCommandAllocatorPool().Create(RenderContext::GetGraphicsDevice());
        GetCommandAllocator(0) = GetCommandAllocatorPool().RequestAllocator(0);
        GetCommandAllocator(1) = GetCommandAllocatorPool().RequestAllocator(1);
        GetCommandQueueManager().CreateCommandQueue(GOHKeys::CmdQueue::PRIMARY);

        auto &primaryCommandQueue = GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY);
        auto &currentCommandAllocator = GetCommandAllocator(m_Display.GetFrameIndex());

        m_Display.GetSwapChain().SetTearingSupport(m_HasTearingSupport);
        m_Display.GetSwapChain().CreateSwapChain(primaryCommandQueue);
        m_Display.TurnOverSwapChainBuffer();



        m_CommandListManager.CreateCommandList(
            GOHKeys::CmdList::INIT,
            currentCommandAllocator,
            true,
            D3D12_COMMAND_LIST_TYPE_DIRECT
        );

        m_CommandListManager.ExecuteCommandList(GOHKeys::CmdList::INIT, primaryCommandQueue);

        m_CommandListManager.CreateCommandList(
            GOHKeys::CmdList::PRIMARY,
            currentCommandAllocator,
            true,
            D3D12_COMMAND_LIST_TYPE_DIRECT
        );

        m_CommandListManager.ExecuteCommandList(GOHKeys::CmdList::PRIMARY, primaryCommandQueue);

        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[3]{};
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

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
        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, RenderContext::GetHighestRootSignatureVersion(), &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            throw std::runtime_error("");
            return;
        }

        result = RenderContext::GetGraphicsDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            throw std::runtime_error("");
            return;
        }

        NAME_D3D12_OBJECT(m_RootSignature);
        m_Scene.InitSceneRenderTargets(
            m_Display.GetFrameIndex(),
            m_Display.GetSwapChain()
        );

        m_Scene.GetDynamicConstantBuffer().Create(RenderContext::GetGraphicsDevice().Get());

        GetPipelineLibrary().Build(RenderContext::GetGraphicsDevice().Get(), m_RootSignature);
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

    ComPointer<ID3D12RootSignature> &Renderer::GetRootSignature()
    {
        return m_RootSignature;
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

    void Renderer::CreateAllocatorPool()
    {
        m_AllocatorPool.Create(RenderContext::GetGraphicsDevice());
    }

    SceneRenderContext::SceneRenderContext(Scene &scene, Display &display)
        : m_Scene(scene)
        , m_Display(display)
    {}

    SceneRenderContext::~SceneRenderContext() = default;

    void SceneRenderContext::FrameStart(ID3D12GraphicsCommandList6 *cmdList, ID3D12RootSignature *rootSignature)
    {
        cmdList->SetGraphicsRootSignature(rootSignature);
        cmdList->RSSetViewports(1, &m_Display.GetViewPort());
        cmdList->RSSetScissorRects(1, &m_Display.GetScissorRect());

        ID3D12DescriptorHeap *ppHeaps[] = { m_Scene.GetSrvHeap() };
        cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        cmdList->SetGraphicsRootConstantBufferView(
            1,
            m_Scene.GetDynamicConstantBuffer().GetGpuVirtualAddress(0, m_Display.GetFrameIndex())
        );
    }

    void SceneRenderContext::ClearRtv(ID3D12GraphicsCommandList6 *cmdList, const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle) const
    {

        cmdList->OMSetRenderTargets(
            1,
            &rtvHandle,
            FALSE,
            nullptr
        );

        cmdList->ClearRenderTargetView(
            rtvHandle,
            INTERMEDIATE_CLEAR_COLOUR,
            0,
            nullptr
        );
    }

    void SceneRenderContext::AddBarrier(uint32 barrierIndex, ID3D12Resource *pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        m_Barriers.emplace_back(barrierIndex, CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after));
    }

    void SceneRenderContext::SetRootDescriptorTable(ID3D12GraphicsCommandList6 *cmdList, uint32 rootParameterIndex)
    {
        cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_Scene.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
    }


    void SceneRenderContext::ExecuteBarrier(ID3D12GraphicsCommandList6 *cmdList, uint32 barrierIndex) const
    {
        for (const auto & [index, barrier] : m_Barriers)
        {
            if (barrierIndex == index)
            {
                cmdList->ResourceBarrier(1, &barrier);
                return;
            }
        }

        RXN_LOGGER::Error(L"No barrier found with index %d", barrierIndex);
    }

    void SceneRenderContext::ExecuteBarriers(ID3D12GraphicsCommandList6 *cmdList) const
    {
        assert(m_Barriers.size() < 256);
        D3D12_RESOURCE_BARRIER barriers[256] = {};
        int i = 0;
        for (const auto &[index, barrier] : m_Barriers)
        {
            barriers[i] = barrier;
            i++;
        }
        cmdList->ResourceBarrier(i, barriers);
    }

    void SceneRenderContext::SwapBarrier(uint32 barrierIndex)
    {

        for (auto &[index, barrier] : m_Barriers)
        {
            if (index == barrierIndex)
            {
                auto newAfterState = barrier.Transition.StateBefore;
                auto newBeforeState = barrier.Transition.StateAfter;
                barrier.Transition.StateBefore = newBeforeState;
                barrier.Transition.StateAfter = newAfterState;
                return;
            }
        }

        RXN_LOGGER::Error(L"No barrier found with index %d", barrierIndex);

    }

    void SceneRenderContext::FrameEnd(ComPointer<ID3D12GraphicsCommandList6> cmdList) const
    {
        cmdList->Close();
    }

}
