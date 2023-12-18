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

    void Renderer::CreateAllocatorPool()
    {
        m_AllocatorPool.Create(RenderContext::GetGraphicsDevice());
    }

   
    RenderPassObjects::RenderPassObjects(Renderer *renderer)
        : m_Renderer(renderer)
    {}

    RenderPassObjects::~RenderPassObjects() = default;

    RenderPassObjects &RenderPassObjects::PopulateNewList(ComPointer<ID3D12GraphicsCommandList> cmdList)
    {
        m_ActiveCmdList = cmdList;
        return *this;
    }

}
