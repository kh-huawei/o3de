/*/
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/Feature/Mesh/MeshInfo.h>
#include <Atom/RHI.Reflect/Handle.h>
#include <Atom/RHI.Reflect/ShaderSemantic.h>
#include <Atom/RHI/BufferView.h>
#include <Atom/RHI/IndexBufferView.h>
#include <Atom/RHI/StreamBufferView.h>
#include <Atom/RPI.Public/Buffer/BufferSystemInterface.h>
#include <Atom/RPI.Public/Model/UvStreamTangentBitmask.h>
#include <Atom/RPI.Reflect/Shader/ShaderOptionGroup.h>


namespace AZ
{
    namespace Render
    {

        using MeshInfoHandle = RHI::Handle<int32_t>;
        using MeshInfoHandleList = AZStd::vector<MeshInfoHandle>;

        // Utility to access the Geometry - Data from a mesh without the input-assembly
        struct BufferViewIndexAndOffset
        {
            // Streambuffer-view and format, needed to build the BLAS
            AZ::RHI::StreamBufferView m_streamBufferView;
            RHI::Format m_streamBufferFormat;

            // Buffer-View, offset and bindless Read index for the same data, needed to access the data with the MeshInfo-indices
            RHI::Ptr<AZ::RHI::BufferView> m_bufferView;
            uint32_t m_byteOffset;
            AZStd::unordered_map<int, uint32_t> m_bindlessReadIndex;

            // utility function to create an entry from a Streambuffer
            static BufferViewIndexAndOffset Create(const RHI::StreamBufferView& streamBufferView, const RHI::Format format)
            {
                BufferViewIndexAndOffset result;
                auto rhiBuffer = streamBufferView.GetBuffer();
                uint32_t byteCount = rhiBuffer->GetDescriptor().m_byteCount;

                RHI::BufferViewDescriptor desc = RHI::BufferViewDescriptor::CreateRaw(0, byteCount);
                if (byteCount > 0)
                {
                    result.m_streamBufferFormat = format;
                    result.m_streamBufferView = streamBufferView;
                    result.m_bufferView = rhiBuffer->BuildBufferView(desc);
                    result.m_bindlessReadIndex = result.m_bufferView->GetBindlessReadIndex();
                    result.m_byteOffset = streamBufferView.GetByteOffset();
                }
                return result;
            };
        };

        // Utility to access the Indices for the Geometry - Data from a mesh without the input-assembly
        struct IndexBufferViewIndexAndOffset
        {
            // Indexbuffer-view and format, needed to build the BLAS
            AZ::RHI::IndexBufferView m_indexBufferView;
            RHI::IndexFormat m_indexBufferFormat;

            // Buffer-View, offset and bindless Read index for the same data, needed to access the data with the MeshInfo-indices
            RHI::Ptr<AZ::RHI::BufferView> m_bufferView;
            uint32_t m_byteOffset;
            AZStd::unordered_map<int, uint32_t> m_bindlessReadIndex;

            // utility function to create an entry from an IndexBufferView
            static IndexBufferViewIndexAndOffset Create(const AZ::RHI::IndexBufferView& indexBufferView)
            {
                // Note: We set up everything so it works for both 16 and 32 bit indices, but on the GPU we use this as a byteaddressbuffer,
                // where we always expect 32 bit.
                AZ_Assert(indexBufferView.GetIndexFormat() == RHI::IndexFormat::Uint32, "Error: only 32-bit indices are supported.");
                RHI::BufferViewDescriptor desc;
                desc.m_elementOffset = 0;
                desc.m_elementSize = AZ::RHI::GetIndexFormatSize(indexBufferView.GetIndexFormat());
                desc.m_elementCount = (uint32_t)indexBufferView.GetBuffer()->GetDescriptor().m_byteCount / desc.m_elementSize;
                desc.m_elementFormat =
                    indexBufferView.GetIndexFormat() == RHI::IndexFormat::Uint16 ? RHI::Format::R16_UINT : RHI::Format::R32_UINT;

                // multi-device buffer bindless read index and offset
                IndexBufferViewIndexAndOffset result{};
                result.m_bufferView = indexBufferView.GetBuffer()->BuildBufferView(desc);
                result.m_bindlessReadIndex = result.m_bufferView->GetBindlessReadIndex();
                result.m_byteOffset = desc.m_elementSize * desc.m_elementOffset;
                result.m_indexBufferView = indexBufferView;
                result.m_indexBufferFormat = indexBufferView.GetIndexFormat();
                return result;
            }
        };

        // Data for for the MeshInfo - entries of one Mesh
        struct MeshInfoEntry : public AZStd::intrusive_base
        {
            // Info from the mesh about the geometry buffers
            RPI::UvStreamTangentBitmask m_streamTangentBitmask;
            RPI::ShaderOptionGroup m_optionalInputStreamShaderOptions;

            // Geometry buffers and index buffer
            AZStd::unordered_map<RHI::ShaderSemantic, BufferViewIndexAndOffset> m_meshBuffers;
            IndexBufferViewIndexAndOffset m_indexBuffer;

            // additional data per mesh
            int32_t m_materialTypeId;
            int32_t m_materialInstanceId;
            uint32_t m_lightingChannels;
            uint32_t m_objectIdForTransform;
        };

    } // namespace Render
} // namespace AZ
