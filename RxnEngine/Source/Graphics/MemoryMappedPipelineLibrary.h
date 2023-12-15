/*****************************************************************//**
 * \file   MemoryMappedPipelineLibrary.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineLibrary : public MemoryMappedFile
    {
    public :

        /**
         * .
         * 
         * \param pDevice
         * \param filename
         * \return 
         */
        bool InitPipelineLibrary(ID3D12Device *pDevice, const WString &filename);

        /**
         * .
         * 
         * \param deleteFile
         */
        void DestroyPipelineLibrary(bool deleteFile);

        /**
         * .
         * 
         * \return 
         */
        ComPointer<ID3D12PipelineLibrary> GetPipelineLibrary();

    private:

        ComPointer<ID3D12PipelineLibrary> m_pipelineLibrary;
    };
}

