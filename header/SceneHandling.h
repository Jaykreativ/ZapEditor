#pragma once

#include "ZapEditor.h"

#include "Zap/Scene/Actor.h"

namespace editor {
	namespace scene {
		void createActor(EditorData& editorData, Zap::Actor actor, std::string name = "");

		void destroyActor(EditorData& editorData, Zap::Actor actor);
	}
}