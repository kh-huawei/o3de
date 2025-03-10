/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/RHI/DrawList.h>
#include <Atom/RHI/DrawPacket.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Public/PipelineState.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Reflect/Material/ShaderCollection.h>
#include <AzCore/std/smart_ptr/intrusive_base.h>

namespace AZ
{
    namespace Render
    {
        // This is a drawpacket with a single fullscreen draw item for one material-type and it's unique set of shader options
        class DeferredDrawPacket : public AZStd::intrusive_base
        {
        public:
            DeferredDrawPacket() = default;
            DeferredDrawPacket(
                const RPI::Scene* scene,
                RPI::Material* material,
                const Name& materialPipelineName,
                const RPI::ShaderCollection::Item& shaderItem,
                const int32_t drawPacketId);

            void CompileDrawSrg(Data::Instance<RPI::Buffer> drawPacketIdBuffer);

            const RHI::DrawPacket* GetRHIDrawPacket()
            {
                return m_drawPacket.get();
            }
            const RHI::DrawPacket* GetRHIDrawPacket() const
            {
                return m_drawPacket.get();
            }
            const RHI::ConstPtr<RHI::ConstantsLayout> GetRootConstantsLayout() const
            {
                return m_rootConstantsLayout;
            }

            RHI::DrawListTag GetDrawListTag() const
            {
                return m_drawListTag;
            }

            int32_t GetDrawPacketId() const
            {
                return m_drawPacketId;
            }

            size_t GetUseCount() const
            {
                return use_count();
            }

            const Data::Asset<RPI::MaterialAsset>& GetInstigatingMaterialAsset() const
            {
                return m_instigatingMaterialAsset;
            }

            // TODO: Who sets shader-options for what?
            // We can only draw multiple meshes with one deferred draw-call if they have the same shader-options,
            // so if these shader-options are somehow linked to a mesh, that mesh needs a different draw-packet.
            // But if these are global (e.g. DebugRendering), this might still be useful
            // bool SetShaderOption(const Name& shaderOptionName, RPI::ShaderOptionValue value);
            // bool UnsetShaderOption(const Name& shaderOptionName);
            // void ClearShaderOptions();

        private:
            void Init(
                const RPI::Scene* scene,
                RPI::Material* material,
                const Name& MaterialPipelineName,
                const RPI::ShaderCollection::Item& shaderItem);

            // unique Id of the draw-packet
            int32_t m_drawPacketId;

            // Non-valid Reference to the material-Asset that was used to create this DeferredDrawPacket.
            // Useful for debugging / logprints, but this should never be used to load the asset
            Data::Asset<RPI::MaterialAsset> m_instigatingMaterialAsset;

            Data::Instance<RPI::Shader> m_shader;
            RPI::ShaderVariantId m_requestedShaderVariantId;
            RPI::ShaderVariantId m_activeShaderVariantId;
            RPI::ShaderVariantStableId m_activeShaderVariantStableId;
            Name m_materialPipelineName;
            Name m_shaderTag;
            RHI::DrawListTag m_drawListTag;
            const RHI::PipelineState* m_pipelineState;
            Data::Instance<RPI::ShaderResourceGroup> m_materialSrg;
            Data::Instance<RPI::ShaderResourceGroup> m_drawSrg;
            RHI::ConstPtr<RHI::ConstantsLayout> m_rootConstantsLayout;
            AZStd::vector<uint8_t> m_rootConstants;
            AZStd::shared_ptr<RHI::GeometryView> m_geometryView;

            RHI::ConstPtr<RHI::DrawPacket> m_drawPacket;

            // TODO: do we need to listen to reloads of the shader?
            // A handler which is called when a shader variant of the material is ready
            // Material::OnMaterialShaderVariantReadyEvent::Handler m_shaderVariantHandler;

            // TODO: do we have global shader-options for deferred materials?
            //! List of shader options set for this specific draw item
            // using AZStd::pair<Name, RPI::ShaderOptionValue> = ShaderOptionPair;
            // using ShaderOptionVector = AZStd::vector<ShaderOptionPair> ShaderOptionVector;
            // ShaderOptionVector m_shaderOptions;
        };

    } // namespace Render
} // namespace AZ