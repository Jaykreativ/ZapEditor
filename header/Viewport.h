#pragma once

#include "VulkanFramework.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "ViewLayer.h"
#include "imgui.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
	class PathTracer;
	class Renderer;
	class Scene;
	class Actor;
}

namespace editor {
	class OutlineRenderTask;
	class DebugRenderTask;

	struct ViewportSettings {
		bool enableOutlines = true;
		bool enableTransformVisual = true;
		bool enablePxDebug = false;
	};

	class Viewport : public ViewLayer
	{
	public:
		bool canMove = true;

		Viewport(Zap::Scene* pScene, Zap::Window* pWindow, std::vector<Zap::Actor>& selectedActors);
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
		ViewportSettings m_settings = {};

		std::vector<Zap::Actor>& m_selectedActors;

		Zap::Window* m_pWindow;
		Zap::Scene* m_pScene;

		Zap::Renderer m_renderer;
		Zap::PBRenderer* m_pPBRender;
		Zap::RaytracingRenderer* m_pRTRender;
		Zap::PathTracer* m_pPathTracer;
		OutlineRenderTask* m_pOutlineRenderTask;
		DebugRenderTask* m_pDebugRenderTask;

		Zap::Actor m_camera;

		RenderType m_renderType = ePBR;
		vk::Image m_outImage;

		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;

		bool m_isFocused = false;
		bool m_isHovered = false;

		double m_xlast = 0;
		double m_ylast = 0;

		vk::Buffer m_debugVertexBuffer;

		void update();

		static void cursorPositionCallback(Zap::CursorPosEvent& params, void* viewportData);
	};
}

