#pragma once

#include "ZapEditor.h"

#include "VulkanFramework.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "ViewLayer.h"
#include "imgui.h"

namespace Zap{
	class PBRenderer;
	class RaytracingRenderer;
	class PathTracer;
	class DebugRenderVertex;
	class DebugRenderTask;
	class Renderer;
	class Scene;
}

namespace editor {
	class OutlineRenderTask;

	struct ViewportSettings {
		bool enableOutlines = true;
		bool enableTransformVisual = true;
		bool enablePxDebug = false;
	};

	class Viewport : public ViewLayer
	{
	public:
		bool canMove = true;

		Viewport(EditorData& editorData, Zap::Scene* pScene, Zap::Window* pWindow);
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

		EditorData& m_editorData;

		std::vector<Zap::Actor>& m_selectedActors;

		Zap::Window* m_pWindow;
		Zap::Scene* m_pScene;

		Zap::Renderer m_renderer;
		Zap::PBRenderer* m_pPBRender = nullptr;
		Zap::RaytracingRenderer* m_pRTRender = nullptr;
		Zap::PathTracer* m_pPathTracer = nullptr;
		OutlineRenderTask* m_pOutlineRenderTask = nullptr;
		Zap::DebugRenderTask* m_pDebugRenderTask = nullptr;

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
		std::vector<Zap::DebugRenderVertex> m_debugLineVector = {};

		Zap::Scene m_transformEditScene;
		Zap::PhysicsMaterial* m_transformMaterial;
		Zap::Actor m_transformX;
		Zap::Actor m_transformY;
		Zap::Actor m_transformZ;
		uint32_t m_axisIndex = 0xFFFFFFFF;
		glm::vec3 m_mousePlanePos = { 0, 0, 0 };
		bool m_isTransformDragged = false;

		void update();

		static void cursorPositionCallback(Zap::CursorPosEvent& params, void* viewportData);

		static void dragDropCallback(Zap::DragDropEvent& params, void* viewportData);
	};
}

