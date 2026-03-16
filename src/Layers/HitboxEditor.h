#pragma once

#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/PBRenderer.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/LineRenderTask.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/Gui.h"

#include "ZapEditor.h"
#include "EditorCamera.h"
#include "ViewLayer.h"

#include <memory>

namespace editor {
	class HitboxEditor : public ViewLayer
	{
	public:
		HitboxEditor(EditorData* pEditorData);
		~HitboxEditor();

		std::string name() override;

		void draw() override;

		ImGuiWindowFlags getWindowFlags() override;

		void updateActor();

		void setupScene();

		void activateRenderer();

		void disableRenderer();

	private:
		EditorData* m_pEditorData;

		std::unique_ptr<Zap::Renderer> m_renderer;
		Zap::RenderTaskHandle<Zap::LineRenderTask> m_lineTask;
		Zap::RenderTaskHandle<Zap::PBRenderer> m_pbrTask;

		Zap::RenderTargetHandle<Zap::RenderTargetGuiImage> m_finalTarget;

		std::shared_ptr<Zap::LineBuffer> m_spLineBuffer;

		std::vector<Zap::Shape> m_shapes;
		std::vector<Zap::Shape> m_editingShapes;
		Zap::Scene m_scene;
		Zap::Actor m_actor;
		Zap::Actor m_oldActor;
		Zap::Actor m_light;
		std::unique_ptr<editor::Camera> m_upCamera;

		Zap::PhysicsMaterial m_physicsMaterial = Zap::PhysicsMaterial(1, 0.5, 0.5);

		bool m_isImageHovered = false;
		bool m_isFocused = false;

		bool m_doesCamTurn;
		float m_sensVert = 0.08;
		float m_sensHorz = 0.05;
		float m_lastX = 0;
		float m_lastY = 0;
		float m_camDist = 5;
	};
}

