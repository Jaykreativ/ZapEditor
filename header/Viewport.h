#pragma once

#include "VulkanFramework.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
	class Gui;
	class Renderer;
}

namespace editor {
	class Viewport
	{
	public:
		bool canMove = true;

		Viewport(Zap::PBRenderer* render, Zap::RaytracingRenderer* rtxRender, Zap::Renderer* renderer);
		~Viewport();

		void draw();

		enum RenderType {
			ePBR = 0,
			eRAYTRACING = 1
		};

		void changeRenderType(RenderType renderType);

		bool isHovered();

	private:
		Zap::PBRenderer* m_pbRender;
		Zap::RaytracingRenderer* m_rtxRender;
		Zap::Renderer* m_renderer;

		RenderType m_renderType = ePBR;
		vk::Image m_outImage;

		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;

		bool m_isHovered = false;

		void update();
	};
}

