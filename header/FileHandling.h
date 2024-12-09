#pragma once

#include "ZapEditor.h"

#include <string>

namespace editor {
	void loadModelFile(std::string filepath, EditorData& editorData);

	void loadSceneFile(std::string filepath, EditorData& editorData);

	void loadActorFile(std::string filepath, EditorData& editorData);

	void loadFile(std::string filepath, EditorData& editorData);
}