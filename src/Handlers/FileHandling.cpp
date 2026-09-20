#include "FileHandling.h"

#include "SceneHandling.h"
#include "ProjectHandling.h"

#include "Zap/Zap.h"
#include "Zap/AssetHandling/Loaders.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Components.h"

#include <iostream>
#include <filesystem>

namespace editor {
	void loadModelFile(std::filesystem::path filepath, EditorData& editorData) {
		Zap::ModelLoader loader;
		loader.load(filepath);
		auto model = loader.result(); // TODO forward drag drop files to the viewlayer its dropped into
	}
	
	//void loadSceneFile(std::filesystem::path filepath, EditorData& editorData) {
	//
	//}
	//
	//void loadActorFile(std::filesystem::path filepath, EditorData& editorData) {
	//	Zap::ActorLoader loader;
	//	scene::createActor(editorData, loader.load(filepath, &editorData.scenes.back()), filepath.filename().replace_extension().string());
	//}
	//
	//void saveActorFile(std::filesystem::path folderpath, Zap::Actor actor, EditorData& editorData) {
	//	std::string name;
	//	if(editorData.actorNameMap.count(actor))
	//		name = editorData.actorNameMap.at(actor);
	//	else
	//		name = std::to_string(actor.getHandle());
	//
	//	std::filesystem::path filepath = folderpath / (name + "." + actorFileExtension);
	//
	//	Zap::ActorLoader loader;
	//	std::cout << "Saving actor to: " << filepath << "\n";
	//	loader.store(filepath, actor);
	//
	//	editorData.actorPathMap[actor] = filepath;
	//}
	//
	void loadFile(std::filesystem::path filepath, EditorData& editorData) {
		std::string fileExtension = filepath.extension().string();
		fileExtension = fileExtension.substr(1, fileExtension.size()-1);// take away the .
		/*if (fileExtension == actorFileExtension) {
			loadActorFile(filepath, editorData);
		}
		else if (fileExtension == sceneFileExtension) {
			loadSceneFile(filepath, editorData);
		}
		else */if(
			fileExtension == "glb" ||
			fileExtension == "gltf" ||
			fileExtension == "obj" )
		{
			loadModelFile(filepath, editorData);
		}
		else if (fileExtension == "zscn") {
			SceneHandler::LoadFailFlags fail;
			auto sceneRef = editorData.pSceneHandler->load(filepath, &fail);
			if (!fail) {
				sceneRef->updatePxPoses();
				sceneRef->update();
			}

		}
		/*else if (
			fileExtension == projectFileExtension ||
			fileExtension == projectEditorFileExtension )
		{
			project::open(editorData, filepath);
		}*/
		else
		{
			ZP_WARN(false, fileExtension + " is not a supported file extension");
		}
	}
}