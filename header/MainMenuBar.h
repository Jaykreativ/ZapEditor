#pragma once

#include "Zap/Rendering/Renderer.h"
#include "Zap/Scene/Scene.h"
#include "Zap/EventHandler.h"

#include "ViewLayer.h"
#include <vector>

namespace editor {
	class MainMenuBar
	{
	public:
		MainMenuBar(
			std::vector<ViewLayer*>& layers,
			Zap::Window* pWindow,
			Zap::Renderer* pRenderer,
			Zap::Scene* pScene,
			std::vector<Zap::Actor>& actors,
			std::vector<Zap::Actor>& selectedActors
		);
		~MainMenuBar();

		void draw();

		bool shouldSimulate();

	private:
		bool m_shouldSimulate = false;
		std::vector<ViewLayer*>& m_layers;
		Zap::Window* m_pWindow;
		Zap::Renderer* m_pRenderer;
		Zap::Scene* m_pScene;
		std::vector<Zap::Actor>& m_actors;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}

