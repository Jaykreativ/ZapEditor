#pragma once

#include "VulkanFramework.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
	class PathTracer;
	class Gui;
	class Renderer;
	class Scene;
	class Actor;
}

namespace editor {
	class Viewport
	{
	public:
		bool canMove = true;

		Viewport(Zap::Renderer* renderer, Zap::Scene* pScene);
		~Viewport();

		void draw(const Zap::Actor& camera);

		enum RenderType {
			ePBR = 0,
			eRAYTRACING = 1,
			ePATHTRACING = 2
		};

		void changeRenderType(RenderType renderType);

		bool isHovered();

	private:
		Zap::PBRenderer* m_pbRender;
		Zap::RaytracingRenderer* m_rtxRender;
		Zap::PathTracer* m_pathTracer;
		Zap::Renderer* m_renderer;

		RenderType m_renderType = ePBR;
		vk::Image m_outImage;

		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;

		bool m_isHovered = false;

		void update();
	};
}

