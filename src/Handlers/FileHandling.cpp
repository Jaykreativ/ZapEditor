#include "FileHandling.h"

#include "SceneHandling.h"
#include "ProjectHandling.h"

#include "Zap/Zap.h"
#include "Zap/FileLoader.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"

#include <iostream>
#include <filesystem>

namespace editor {
	void loadModelFile(std::filesystem::path filepath, EditorData& editorData) {
		std::string filename = filepath.filename().string();

		Zap::ModelLoader loader;
		auto model = loader.load(filepath);

		editorData.actors.push_back(Zap::Actor());
		Zap::Actor& actor = editorData.actors.back();
		editorData.scenes.back().attachActor(actor);
		editorData.actorNameMap[actor] = filename;
		actor.addTransform(glm::mat4(1));
		actor.addModel(model);
	}

	void loadSceneFile(std::filesystem::path filepath, EditorData& editorData) {

	}

	void loadActorFile(std::filesystem::path filepath, EditorData& editorData) {
		Zap::ActorLoader loader;
		scene::createActor(editorData, loader.load(filepath, &editorData.scenes.back()), filepath.filename().replace_extension().string());
	}

	void saveActorFile(std::filesystem::path folderpath, Zap::Actor actor, EditorData& editorData) {
		std::string name;
		if(editorData.actorNameMap.count(actor))
			name = editorData.actorNameMap.at(actor);
		else
			name = std::to_string(actor.getHandle());

		std::filesystem::path filepath = folderpath / (name + "." + actorFileExtension);

		Zap::ActorLoader loader;
		std::cout << "Saving actor to: " << filepath << "\n";
		loader.store(filepath, actor);

		editorData.actorPathMap[actor] = filepath;
	}

	void loadFile(std::filesystem::path filepath, EditorData& editorData) {
		std::string fileExtension = filepath.extension().string();
		fileExtension = fileExtension.substr(1, fileExtension.size()-1);// take away the .
		if (fileExtension == actorFileExtension) {
			loadActorFile(filepath, editorData);
		}
		else if (fileExtension == sceneFileExtension) {
			loadSceneFile(filepath, editorData);
		}
		else if(
			fileExtension == "glb" ||
			fileExtension == "obj" )
		{
			loadModelFile(filepath, editorData);
		}
		else if (
			fileExtension == projectFileExtension ||
			fileExtension == projectEditorFileExtension )
		{
			project::open(editorData, filepath);
		}
		else
		{
			ZP_WARN(false, fileExtension + " is not a supported file extension");
		}
	}
}