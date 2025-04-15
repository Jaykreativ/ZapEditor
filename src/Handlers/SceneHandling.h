#pragma once

#include "ZapEditor.h"

#include "Zap/Scene/Actor.h"

namespace editor {
	namespace scene {
		Zap::Scene& createScene(EditorData& editorData);

		void destroyScene(EditorData& editorData, Zap::Scene& scene);
		void destroyScene(EditorData& editorData, uint32_t sceneIndex);

		void selectScene(EditorData& editorData, Zap::Scene& scene);
		void selectScene(EditorData& editorData, uint32_t sceneIndex);

		void createActor(EditorData& editorData, Zap::Actor actor, std::string name = "");

		void destroyActor(EditorData& editorData, Zap::Actor actor);
		void destroyActor(EditorData& editorData, uint32_t actorIndex);
	}
}