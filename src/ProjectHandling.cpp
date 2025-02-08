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
			project.fileDir = directory + "/" + name + "." + projectFileExtension;
			project.editorFileDir = directory + "/" + name + "." + projectEditorFileExtension;

			// read contents of *.zproj
			bool success = serializer.beginDeserialization(project.fileDir.c_str());
			if (success) {

				project.name = serializer.readAttribute("name", &success);

				// Load AssetLibrary
				{
					bool s = true; // doesn't count towards load failing has its own error handling
					project.assetLibraryPath = serializer.readAttribute("assetLibraryPath", &s);
					if (s)
						editorData.engineBase->getAssetHandler()->loadFromFile(project.rootPath + "/" + project.assetLibraryPath);
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
					loadActorFile(path, editorData);
				}
				serializer.endElement();

			}
			serializer.endDeserialization();
			ZP_WARN(success, ("Failed to load project file: " + project.fileDir).c_str());

			// read contents of *.zproj.edit
			success = success && serializer.beginDeserialization(project.editorFileDir.c_str());
			if (success) {

			}
			serializer.endDeserialization();
			ZP_WARN(success, ("Failed to load project file: " + project.editorFileDir).c_str());

			return success;
		}
		bool readFiles(EditorData& editorData, std::string filepath) {
			std::string directory = "";
			std::string name = "";
			std::string extension = "";

			seperatePath(filepath, directory, name, extension);

			return readFiles(editorData, name, directory);
		}

		// writes all project information to file
		bool writeFiles(EditorData& editorData) {
			ProjectData& project = editorData.project;
			Zap::Serializer serializer;

			serializer.beginSerialization(project.fileDir.c_str());

			serializer.writeAttribute("name", project.name);

			// save AssetLibrary
			serializer.writeAttribute("assetLibraryPath", project.assetLibraryPath);
			editorData.engineBase->getAssetHandler()->saveToFile(project.rootPath + "/" + project.assetLibraryPath);

			// save actors
			for (auto actor : editorData.actors) {
				saveActorFile("Actors", actor, editorData);
			}

			serializer.beginElement("ActorPaths");
			serializer.writeAttribute("actorPathCount", editorData.actorPathMap.size());
			size_t i = 0;
			for (auto& actorPathPair : editorData.actorPathMap) {
				serializer.writeAttribute("actorPath" + std::to_string(i), actorPathPair.second);
				i++;
			}
			serializer.endElement();

			serializer.endSerialization();

			serializer.beginSerialization(project.editorFileDir.c_str());
			serializer.endSerialization();

			return true;
		}

		void create(EditorData& editorData, std::string name, std::string directory) {
			close(editorData);

			// create project data
			ProjectData& project = editorData.project;
			project.name = name;
			project.rootPath = directory;
			project.fileDir = directory + "/" + project.name + "." + projectFileExtension;
			project.editorFileDir = directory + "/" + project.name + "." + projectEditorFileExtension;
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
		void open(EditorData& editorData, std::string filepath) {
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