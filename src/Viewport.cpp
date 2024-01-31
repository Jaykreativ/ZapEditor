#include "Viewport.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/RaytracingRenderer.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace editor {
	Viewport::Viewport(Zap::PBRenderer* render, Zap::RaytracingRenderer* rtxRender, Zap::Renderer* renderer)
		: m_render(render), m_rtxRender(rtxRender), m_renderer(renderer)
	{
		m_rtxOutImage = &m_rtxRender->getOutputImage();

		m_rtxOutSampler.init();

		m_rtxOutDescriptorSet = ImGui_ImplVulkan_AddTexture(m_rtxOutSampler, m_rtxOutImage->getVkImageView(), m_rtxOutImage->getLayout());
	}

	Viewport::~Viewport() {
		m_rtxOutSampler.destroy();
	}

	void Viewport::draw() {
		auto imageExtent = m_rtxOutImage->getExtent();
		auto extent = ImGui::GetContentRegionAvail();
		if (extent.x != imageExtent.width || extent.y != imageExtent.height) {
			ImGui_ImplVulkan_RemoveTexture(m_rtxOutDescriptorSet);
			m_rtxRender->resize(extent.x, extent.y);
			m_renderer->update();
			m_rtxOutDescriptorSet = ImGui_ImplVulkan_AddTexture(m_rtxOutSampler, m_rtxOutImage->getVkImageView(), m_rtxOutImage->getLayout());
		}
		ImGui::Image(m_rtxOutDescriptorSet, extent);
		m_isHovered = ImGui::IsItemHovered();
	}

	bool Viewport::isHovered() {
		return m_isHovered;
	}
}