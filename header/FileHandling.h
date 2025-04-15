#pragma once

#include "ZapEditor.h"

#include <string>
#include <filesystem>

namespace editor {
	void loadModelFile(std::filesystem::path filepath, EditorData& editorData);

	void loadSceneFile(std::filesystem::path filepath, EditorData& editorData);

	void loadActorFile(std::filesystem::path filepath, EditorData& editorData);

	void saveActorFile(std::filesystem::path folderpath, Zap::Actor actor, EditorData& editorData);

	void loadFile(std::filesystem::path filepath, EditorData& editorData);
}