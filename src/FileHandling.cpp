#include "FileHandling.h"

#include "SceneHandling.h"
#include "ProjectHandling.h"

#include "Zap/Zap.h"
#include "Zap/FileLoader.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"

#include <iostream>

namespace editor {
	std::string cleanFolderpath(std::string folderpath) {
		auto size = folderpath.size();
		int lastSlash = folderpath.find_last_of('/');
		if (lastSlash == std::string::npos)
			lastSlash = -1;
		int lastBackslash = folderpath.find_last_of('\\');
		if (lastBackslash == std::string::npos)
			lastBackslash = -1;

		if (std::max(lastSlash, lastBackslash) == size - 1)
			return folderpath;

		if (lastBackslash >= 0)
			return folderpath + "\\";
		else
			return folderpath + "/";
	}

	void seperatePath(std::string filepath, std::string& directory, std::string& name, std::string& extension) {
		auto end = filepath.size();
		auto dot = std::min<size_t>(filepath.find_first_of('.'), end);
		auto extensionFirst = std::min<size_t>(dot+1, end);
		auto slash = filepath.find_last_of('/');
		auto backslash = filepath.find_last_of('\\');
		if (slash >= end) slash = 0;
		if (backslash >= end) backslash = 0;
		auto nameFirst = std::min<size_t>(std::max<size_t>(slash, backslash) + 1, dot);
		auto dirLast = std::max<size_t>(nameFirst - 1, 0);

		directory = filepath.substr(0, dirLast);
		name = filepath.substr(nameFirst, dot - nameFirst);
		extension = filepath.substr(extensionFirst, end - dot);
	}

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
		std::string directory = "";
		std::string name = "";
		std::string extension = "";

		seperatePath(filepath, directory, name, extension);

		Zap::ActorLoader loader;
		scene::createActor(editorData, loader.load(filepath, &editorData.scenes.back()), name);
	}

	void saveActorFile(std::string folderpath, Zap::Actor actor, EditorData& editorData) {
		std::string name;
		if(editorData.actorNameMap.count(actor))
			name = editorData.actorNameMap.at(actor);
		else
			name = std::to_string(actor.getHandle());

		folderpath = cleanFolderpath(folderpath);
		std::string filepath = folderpath + name + "." + actorFileExtension;

		Zap::ActorLoader loader;
		std::cout << "Saving actor to: " << filepath << "\n";
		loader.store(filepath, actor);

		editorData.actorPathMap[actor] = filepath;
	}

	void loadFile(std::string filepath, EditorData& editorData) {
		size_t dotIndex = filepath.find_last_of('.');
		std::string fileExtension = filepath.substr(dotIndex+1, filepath.size()-dotIndex);
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