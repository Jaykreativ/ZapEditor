#include "Viewport.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/Rendering/Gui.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace editor {
	Viewport::Viewport(Zap::PBRenderer* pbRender, Zap::RaytracingRenderer* rtxRender, Zap::Renderer* renderer)
		: m_pbRender(pbRender), m_rtxRender(rtxRender), m_renderer(renderer)
	{
		m_outImage.setFormat(Zap::GlobalSettings::getColorFormat());
		m_outImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_outImage.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_outImage.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_outImage.setWidth(1);
		m_outImage.setHeight(1);
		
		m_outImage.init();
		m_outImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_outImage.initView();

		m_pbRender->setRenderTarget(&m_outImage);
		m_rtxRender->setRenderTarget(&m_outImage);

		m_sampler.init();

		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);
	}

	Viewport::~Viewport() {
		m_sampler.destroy();
		m_outImage.destroy();
	}

	void Viewport::draw() {
		auto imageExtent = m_outImage.getExtent();
		auto extent = ImGui::GetContentRegionAvail();
		if (extent.x != imageExtent.width || extent.y != imageExtent.height) {// resize
			m_outImage.setWidth(extent.x);
			m_outImage.setHeight(extent.y);
			if (m_renderType == ePBR) {
				m_pbRender->setViewport(extent.x, extent.y, 0, 0);
			}
			update();
		}
		ImGui::Image(m_imageDescriptorSet, extent);
		m_isHovered = ImGui::IsItemHovered();
	}

	void Viewport::changeRenderType(RenderType renderType) {
		m_renderType = renderType;
		update();
	}

	bool Viewport::isHovered() {
		return m_isHovered;
	}

	void Viewport::update() {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		m_outImage.update();
		m_renderer->beginRecord();
		switch (m_renderType)
		{
		case ePBR:
			m_pbRender->resize();
			m_renderer->recRenderTemplate(m_pbRender);
			m_renderer->recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
			break;
		case eRAYTRACING:
			m_rtxRender->resize();
			m_renderer->recRenderTemplate(m_rtxRender);
			break;
		default:
			break;
		}
		m_renderer->endRecord();
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

	}
}