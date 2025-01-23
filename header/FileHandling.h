#pragma once

#include "ZapEditor.h"

#include <string>

namespace editor {
	std::string cleanFolderpath(std::string folderpath);

	// seperates the different parts of a filepath
	// output is written to the directory, name and extension references
	// Example: dir/image.png -> dir, image, png
	void seperatePath(std::string filepath, std::string& directory, std::string& name, std::string& extension);

	void loadModelFile(std::string filepath, EditorData& editorData);

	void loadSceneFile(std::string filepath, EditorData& editorData);

	void loadActorFile(std::string filepath, EditorData& editorData);

	void saveActorFile(std::string folderpath, Zap::Actor actor, EditorData& editorData);

	void loadFile(std::string filepath, EditorData& editorData);
}