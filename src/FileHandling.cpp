#include "FileHandling.h"

#include "Zap/Zap.h"
#include "Zap/FileLoader.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"

#include <iostream>

namespace editor {
	void loadModelFile(std::string filepath, EditorData& editorData) {
		auto first = std::min(filepath.find_last_of("\\"), filepath.size() - 1)+1;
		auto last = std::min(filepath.find_first_of("."), filepath.size() - 1);
		std::string filename = filepath.substr(first, last - first);

		Zap::ModelLoader loader;
		auto model = loader.load(filepath);

		editorData.actors.push_back(Zap::Actor());
		Zap::Actor& actor = editorData.actors.back();
		editorData.scenes.back().attachActor(actor);
		editorData.actorNameMap[actor] = filename;
		actor.addTransform(glm::mat4(1));
		actor.addModel(model);
	}

	void loadSceneFile(std::string filepath, EditorData& editorData) {

	}

	void loadActorFile(std::string filepath, EditorData& editorData) {

	}

	void loadFile(std::string filepath, EditorData& editorData) {
		size_t dotIndex = filepath.find_last_of('.');
		std::string fileExtension = filepath.substr(dotIndex+1, filepath.size()-dotIndex);
		if (fileExtension == "zac") {
			loadActorFile(filepath, editorData);
		}
		else if (fileExtension == "zsc") {
			loadSceneFile(filepath, editorData);
		}
		else if(
			fileExtension == "glb" ||
			fileExtension == "obj"
			)
		{
			loadModelFile(filepath, editorData);
		}
		else
		{
			ZP_WARN(false, fileExtension + " is not a supported file extension");
		}
	}
}