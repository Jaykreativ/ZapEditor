#include "SceneHandling.h"

namespace editor {
	namespace scene {
		Zap::Scene& createScene(EditorData& editorData) {
			editorData.scenes.push_back(Zap::Scene());
			return editorData.scenes.back();
		}

		void destroyScene(EditorData& editorData, Zap::Scene& scene) {
			uint32_t i = 0;
			for (auto& s : editorData.scenes) {
				if (s == scene)
					destroyScene(editorData, scene);
				i++;
			}
		}
		void destroyScene(EditorData& editorData, uint32_t sceneIndex) {
			editorData.scenes[sceneIndex].destroy(); // TODO delete all actors inside the scene
			editorData.scenes.erase(editorData.scenes.begin() + sceneIndex);
		}

		void selectScene(EditorData& editorData, Zap::Scene& scene) {
			uint32_t i = 0;
			for (auto& s : editorData.scenes) {
				if (s == scene)
					selectScene(editorData, i);
				i++;
			}
		}
		void selectScene(EditorData& editorData, uint32_t sceneIndex) {
			editorData.pActiveScene = &editorData.scenes[sceneIndex];
		}

		void createActor(EditorData& editorData, Zap::Actor actor, std::string name) {
			editorData.actors.push_back(actor);
			if (name == "")
				name = std::to_string(actor.getHandle());
			editorData.actorNameMap[actor] = name;
		}

		void destroyActor(EditorData& editorData, Zap::Actor actor) {
			uint32_t i = 0;
			for (auto& a : editorData.actors) {
				if (a == actor)
					destroyActor(editorData, i);
				i++;
			}
		}
		void destroyActor(EditorData& editorData, uint32_t actorIndex) {
			//delete custom data
			if (editorData.actorNameMap.count(editorData.actors[actorIndex]))
				editorData.actorNameMap.erase(editorData.actors[actorIndex]);
			//delete actor
			editorData.actors[actorIndex].destroy();
			editorData.actors.erase(editorData.actors.begin() + actorIndex);
		}
	}
}
