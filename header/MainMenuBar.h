#pragma once

#include "ZapEditor.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "Zap/EventHandler.h"

#include "ViewLayer.h"
#include <vector>

namespace editor {
	class MainMenuBar
	{
	public:
		MainMenuBar(
			EditorData* pEditorData,
			std::vector<ViewLayer*>& layers,
			Zap::Window* pWindow,
			Zap::Renderer* pRenderer,
			Zap::Gui* pGui,
			Zap::Scene* pScene,
			std::vector<Zap::Actor>& actors,
			std::vector<Zap::Actor>& selectedActors
		);
		~MainMenuBar();

		void draw();

		bool shouldSimulate();

	private:
		bool m_shouldSimulate = false;
		EditorData* m_pEditorData;
		std::vector<ViewLayer*>& m_layers;
		Zap::Window* m_pWindow;
		Zap::Renderer* m_pRenderer;
		Zap::Scene* m_pScene;
		Zap::Gui* m_pGui;
		std::vector<Zap::Actor>& m_actors;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}

