#pragma once

#include "VulkanFramework.h"
#include "Zap/Scene/Actor.h"
#include "ViewLayer.h"
#include "imgui.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
	class PathTracer;
	class Gui;
	class EventHandler;
	class Renderer;
	class Scene;
	class Actor;
}

namespace editor {
	class Viewport : public ViewLayer
	{
	public:
		bool canMove = true;

		Viewport(Zap::Renderer* renderer, Zap::Scene* pScene, Zap::EventHandler* eventHandler);
		~Viewport();

		std::string name();

		void move(float dTime);

		void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

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
		Zap::PBRenderer* m_pbRender;
		Zap::RaytracingRenderer* m_rtxRender;
		Zap::PathTracer* m_pathTracer;
		Zap::Renderer* m_pRenderer;
		Zap::EventHandler* m_pEventHandler;

		Zap::Actor m_camera;

		RenderType m_renderType = ePBR;
		vk::Image m_outImage;

		vk::Sampler m_sampler;
		VkDescriptorSet m_imageDescriptorSet;

		bool m_isHovered = false;

		void update();
	};
}

