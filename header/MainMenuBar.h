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
			Zap::Renderer* pRenderer,
			Zap::Scene* pScene,
			Zap::EventHandler* pEventHandler,
			std::vector<Zap::Actor>& actors,
			std::vector<Zap::Actor>& selectedActors
		);
		~MainMenuBar();

		void draw();

		bool shouldSimulate();

	private:
		bool m_shouldSimulate = false;
		std::vector<ViewLayer*>& m_layers;
		Zap::Renderer* m_pRenderer;
		Zap::Scene* m_pScene;
		Zap::EventHandler* m_pEventHandler;
		std::vector<Zap::Actor>& m_actors;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}

