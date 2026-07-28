#pragma once

#include "Handlers/SceneHandling.h"

#include "imgui.h"
#include <string>

namespace editor {
	class ViewLayer {
	public:
		ViewLayer() = default;

		virtual ~ViewLayer() = default;

		virtual std::string name() = 0;

		virtual void draw() = 0;
		
		virtual ImGuiWindowFlags getWindowFlags() = 0;
	};

	class SceneAccessLayer : public ViewLayer {
	public:
		SceneAccessLayer(SceneHandler& sceneHandler);
		virtual ~SceneAccessLayer() = default;

		virtual void draw();

	protected:
		SceneReference& scene();

	private:
		SceneHandler& m_sceneHandler;
		std::unique_ptr<SceneReference> m_sceneRef;
		bool m_isActive = true;
	};
}
