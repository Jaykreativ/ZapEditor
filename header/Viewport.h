#pragma once

#include "VulkanFramework.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/PathTacer.h"
#include "ViewLayer.h"
#include "imgui.h"

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
	class Viewport : public ViewLayer
	{
	public:
		bool canMove = true;

		Viewport(Zap::Scene* pScene, Zap::Window* pWindow);
		~Viewport();

		std::string name();

		void move(float dTime);

		void draw();

		enum RenderType {
			ePBR = 0,
			eRAYTRACING = 1,
			ePATHTRACING = 2
		};

		ImGuiWindowFlags getWindowFlags();

		void changeRenderType(RenderType renderType);

		bool isHovered() { return m_isHovered; }

	private:
		Zap::Window* m_pWindow;
		Zap::Renderer m_renderer;
		Zap::PBRenderer* m_pPBRender;
		Zap::RaytracingRenderer* m_pRTRender;
		Zap::PathTracer* m_pPathTracer;
		Zap::Actor m_camera;

		RenderType m_renderType = ePBR;
		vk::Image m_outImage;

		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;

		bool m_isHovered = false;

		void update();

		static void cursorPositionCallback(Zap::CursorPosEvent& params, void* viewportData);
	};
}

