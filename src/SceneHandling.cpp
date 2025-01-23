#include "SceneHandling.h"

namespace editor {
	namespace scene {
		void createActor(EditorData& editorData, Zap::Actor actor, std::string name) {
			editorData.actors.push_back(actor);
			if (name == "")
				name = std::to_string(actor.getHandle());
			editorData.actorNameMap[actor] = name;
		}

		void destroyActor(EditorData& editorData, Zap::Actor actor) {

		}
	}
}
