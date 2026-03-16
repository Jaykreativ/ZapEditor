#pragma once

#include "ZapEditor.h"

#include "VulkanFramework.h"

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/LineRenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/DeferredShading.h"

#include "ViewLayer.h"
#include "EditorCamera.h"
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

		void draw();

		enum RenderType {
			ePBR = 0,
			eRAYTRACING = 1,
			ePATHTRACING = 2,
			eDEFERRED = 3
		};

		ImGuiWindowFlags getWindowFlags();

		void activatePBR();

		void activatePathTracer();

		void disableRenderType();

		bool isHovered() { return m_isHovered; }

	private:
		ViewportSettings m_settings = {};

		EditorData& m_editorData;

		std::vector<Zap::Actor>& m_selectedActors;

		Zap::Window* m_pWindow;
		Zap::Scene* m_pScene;

		RenderType m_renderType = ePBR;
		std::unique_ptr<Zap::Renderer> m_renderer;
		Zap::RenderTaskHandle<OutlineRenderTask> m_outlineTask;
		Zap::RenderTaskHandle<Zap::LineRenderTask> m_lineTask;
		Zap::RenderTaskHandle<Zap::PBRenderer> m_pbrTask;
		Zap::RenderTaskHandle<Zap::PathTracer> m_pathTraceTask;

		Zap::RenderTargetHandle<Zap::RenderTargetGuiImage> m_finalTarget;

		editor::Camera m_camera;

		bool m_isFocused = false;
		bool m_isHovered = false;

		double m_xlast = 0;
		double m_ylast = 0;

		std::shared_ptr<Zap::LineBuffer> m_spLineBuffer;

		Zap::Scene m_transformEditScene;
		Zap::PhysicsMaterial* m_transformMaterial;
		Zap::Actor m_transformX;
		Zap::Actor m_transformY;
		Zap::Actor m_transformZ;
		uint32_t m_axisIndex = 0xFFFFFFFF;
		glm::vec3 m_mousePlanePos = { 0, 0, 0 };
		bool m_isTransformDragged = false;
	};
}

