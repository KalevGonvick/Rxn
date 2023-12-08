#include "Rxn.h"
#include "MemoryMappedPipelineLibrary.h"

namespace Rxn::Graphics::Mapped
{
    bool MemoryMappedPipelineLibrary::InitPipelineLibrary(ID3D12Device *pDevice, const WString &filename)
    {
        if (pDevice)
        {
            RXN_LOGGER::Debug(L"Device exists... creating pipeline library device.");


            // Create the Pipeline Library.
            ComPointer<ID3D12Device1> device1;
            if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&device1))))
            {

                RXN_LOGGER::Debug(L"Initializing memory mapped file: %s", filename.c_str());
                InitFile(filename);

                RXN_LOGGER::Debug(L"Create a Pipeline Library from the serialized blob.");
                RXN_LOGGER::Debug(L"Note: The provided Library Blob must remain valid for the lifetime of the object returned - for efficiency, the data is not copied.");
                switch (const HRESULT hr = device1->CreatePipelineLibrary(GetData(), GetSize(), IID_PPV_ARGS(&m_pipelineLibrary)))
                {
                case DXGI_ERROR_UNSUPPORTED:
                {
                    break;
                }
                case E_INVALIDARG:                          // The provided Library is corrupted or unrecognized.
                case D3D12_ERROR_ADAPTER_NOT_FOUND:         // The provided Library contains data for different hardware (Don't really need to clear the cache, could have a cache per adapter).
                case D3D12_ERROR_DRIVER_VERSION_MISMATCH:   // The provided Library contains data from an old driver or runtime. We need to re-create it.
                {
                    DestroyFile(true);
                    InitFile(filename);
                    ThrowIfFailed(device1->CreatePipelineLibrary(GetData(), GetSize(), IID_PPV_ARGS(&m_pipelineLibrary)));
                    break;
                }

                default:
                {
                    ThrowIfFailed(hr);
                }
                }

                if (m_pipelineLibrary)
                {
                    NAME_D3D12_OBJECT(m_pipelineLibrary);
                }
            }
        }

        return m_pipelineLibrary != nullptr;
    }

    void MemoryMappedPipelineLibrary::DestroyPipelineLibrary(bool deleteFile)
    {
        // If we're not going to destroy the file, serialize the library to disk.
        if (!deleteFile && m_pipelineLibrary)
        {
            // Important: An ID3D12PipelineLibrary object becomes undefined when the underlying memory, that was used to initalize it, changes.

            assert(m_pipelineLibrary->GetSerializedSize() <= UINT_MAX);    // Code below casts to UINT.
            const auto librarySize = static_cast<uint32>(m_pipelineLibrary->GetSerializedSize());
            if (librarySize > 0)
            {
                // Grow the file if needed.
                const size_t neededSize = sizeof(UINT) + librarySize;
                void *pTempData = new BYTE[librarySize];
                if (neededSize > GetCurrentFileSize() && pTempData)
                {
                    ThrowIfFailed(m_pipelineLibrary->Serialize(pTempData, librarySize));

                    // Now it's safe to grow the mapping.
                    MemoryMappedFile::GrowMapping(librarySize);

                    // Save the size of the library and the library itself.
                    memcpy(GetData(), pTempData, librarySize);
                    MemoryMappedFile::SetSize(librarySize);

                    delete[] pTempData;
                    pTempData = nullptr;
                    
                }
                else
                {
                    // The mapping didn't change, we can serialize directly to the mapped file.
                    // Save the size of the library and the library itself.
                    assert(neededSize <= GetCurrentFileSize());
                    ThrowIfFailed(m_pipelineLibrary->Serialize(GetData(), librarySize));
                    MemoryMappedFile::SetSize(librarySize);
                }

                // m_pipelineLibrary is now undefined because we just wrote to the mapped file, don't use it again.
                // This is ok in this sample because we only write to the mapped file when the sample exits.
            }
        }

        DestroyFile(deleteFile);
        m_pipelineLibrary = nullptr;
    }

    ID3D12PipelineLibrary *MemoryMappedPipelineLibrary::GetPipelineLibrary()
    {
        return m_pipelineLibrary.Get();
    }
}
