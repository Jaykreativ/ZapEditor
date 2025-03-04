#include "ProjectHandling.h"

#include "FileHandling.h"
#include "SceneHandling.h"

#include "Zap/Serializer.h"

namespace editor {
	namespace project {
		// reads all project information from file into the editorData
		bool readFiles(EditorData& editorData, std::string name, std::string directory) {
			ProjectData& project = editorData.project;

			Zap::Settings* engineSettings = editorData.engineBase->getSettings();

			Zap::Serializer serializer;

			project.rootPath = directory;
			project.projectFile = directory + "/" + name + "." + projectFileExtension;
			project.editorFile = directory + "/" + name + "." + projectEditorFileExtension;

			// read contents of *.zproj
			bool success = serializer.beginDeserialization(project.projectFile.c_str());
			if (success) {

				project.name = serializer.readAttribute("name", &success);

				// Load AssetLibrary
				{
					bool s = true; // doesn't count towards load failing has its own error handling
					project.assetLibraryPath = serializer.readAttribute("assetLibraryPath", &s);
					if (s)
						editorData.engineBase->getAssetHandler()->loadFromFile(project.rootPath / project.assetLibraryPath);
					else {
						ZP_WARN(false, "loaded Project has no AssetLibrary");
						success = false;
					}
				}

				// Load all actors
				serializer.beginElement("ActorPaths");
				size_t actorPathCount = serializer.readAttributeull("actorPathCount", &success);
				for (uint32_t i = 0; i < actorPathCount; i++) {
					std::string path = serializer.readAttribute("actorPath" + std::to_string(i), &success);
					loadActorFile(project.rootPath / path, editorData);
					printf("loading actor: %s\n", path.c_str());
				}
				serializer.endElement();

			}
			serializer.endDeserialization();
			ZP_WARN(success, ("Failed to load project file: " + project.rootPath.string()).c_str());

			// read contents of *.zproj.edit
			success = success && serializer.beginDeserialization(project.editorFile);
			if (success) {

			}
			serializer.endDeserialization();
			ZP_WARN(success, ("Failed to load project file: " + project.editorFile.string()).c_str());

			return success;
		}
		bool readFiles(EditorData& editorData, std::string filepath) {
			return readFiles(editorData, filepath);
		}
		bool readFiles(EditorData& editorData, std::filesystem::path filepath) {
			return readFiles(editorData, filepath.filename().replace_extension().string(), filepath.parent_path().string());
		}

		// writes all project information to file
		bool writeFiles(EditorData& editorData) {
			ProjectData& project = editorData.project;
			Zap::Serializer serializer;

			serializer.beginSerialization(project.projectFile.c_str());

			serializer.writeAttribute("name", project.name);

			// save AssetLibrary
			serializer.writeAttribute("assetLibraryPath", project.assetLibraryPath.string());
			editorData.engineBase->getAssetHandler()->saveToFile(project.rootPath / project.assetLibraryPath);

			// save actors
			for (auto actor : editorData.actors) {
				saveActorFile(project.rootPath / "Actors", actor, editorData);// TODO set default location of actor files in editor
			}

			serializer.beginElement("ActorPaths");
			serializer.writeAttribute("actorPathCount", editorData.actorPathMap.size());
			size_t i = 0;
			for (auto& actorPathPair : editorData.actorPathMap) {
				//save with path relative to the root
				//actorPathPair.second
				serializer.writeAttribute("actorPath" + std::to_string(i), actorPathPair.second.string());
				i++;
			}
			serializer.endElement();

			serializer.endSerialization();

			serializer.beginSerialization(project.editorFile);
			serializer.endSerialization();

			return true;
		}

		void create(EditorData& editorData, std::string name, std::string directory) {
			close(editorData);

			// create project data
			ProjectData& project = editorData.project;
			project.name = name;
			project.rootPath = directory;
			project.projectFile = directory + "/" + project.name + "." + projectFileExtension;
			project.editorFile = directory + "/" + project.name + "." + projectEditorFileExtension;
			project.assetLibraryPath = project.name + "." + assetLibraryFileExtension;


			save(editorData);

			project.isOpen = true;

		}

		void open(EditorData& editorData, std::string name, std::string directory) {
			close(editorData);
			if (!readFiles(editorData, name, directory)) {
				close(editorData);
				return;
			}

			editorData.project.isOpen = true;
		}
		void open(EditorData& editorData, std::filesystem::path filepath) {
			close(editorData);
			if (!readFiles(editorData, filepath)) {
				close(editorData);
				return;
			}
			editorData.project.isOpen = true;
		}

		void close(EditorData& editorData) {
			for (auto& actor : editorData.actors) {// TODO make a flag that automatically saves the project on close
				scene::destroyActor(editorData, 0);
			}
			editorData.engineBase->getAssetHandler()->destroyAssets();
			ProjectData& project = editorData.project;
			project = {};
		}

		void save(EditorData& editorData) {
			writeFiles(editorData);
		}
	}
}