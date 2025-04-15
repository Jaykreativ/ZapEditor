#pragma once

#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/DebugRenderTask.h"
#include "Zap/Rendering/Gui.h"

#include "ZapEditor.h"
#include "ViewLayer.h"

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

	private:
		EditorData* m_pEditorData;

		Zap::GuiImage m_outImage;
		Zap::Renderer m_renderer;
		Zap::PBRenderer m_pbrTask;
		vk::Buffer m_debugVertexBuffer;
		Zap::DebugRenderTask m_debugTask;

		Zap::PhysicsMaterial m_physicsMaterial = Zap::PhysicsMaterial(1, 0.5, 0.5);

		std::vector<Zap::Shape> m_shapes;
		std::vector<Zap::Shape> m_editingShapes;
		Zap::Scene m_scene;
		Zap::Actor m_actor;
		Zap::Actor m_oldActor;
		Zap::Actor m_light;
		Zap::Actor m_camera;

		bool m_isImageHovered = false;

		bool m_doesCamTurn;
		float m_sensVert = 0.08;
		float m_sensHorz = 0.05;
		float m_lastX = 0;
		float m_lastY = 0;
		float m_camDist = 5;
	};
}

