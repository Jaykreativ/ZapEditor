#pragma once

#include "VulkanFramework.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
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

		bool isHovered();

	private:
		Zap::PBRenderer* m_render;
		Zap::RaytracingRenderer* m_rtxRender;
		Zap::Renderer* m_renderer;

		vk::Image* m_rtxOutImage;
		vk::Sampler m_rtxOutSampler;
		VkDescriptorSet m_rtxOutDescriptorSet;

		bool m_isHovered = false;
	};
}

