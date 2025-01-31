#include "ProjectHandling.h"

#include "FileHandling.h"
#include "SceneHandling.h"

#include "Zap/Serializer.h"

namespace editor {
	namespace project {
		// reads all project information from file into the editorData
		bool readFiles(EditorData& editorData, std::string name, std::string directory) {
			ProjectData& project = editorData.project;
			Zap::Serializer serializer;

			project.fileDir = directory + "/" + name + "." + projectFileExtension;
			project.editorFileDir = directory + "/" + name + "." + projectEditorFileExtension;

			// read contents of *.zproj
			bool success = serializer.beginDeserialization(project.fileDir.c_str());
			if (success) {

				project.name = serializer.readAttribute("name", &success);

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
			ProjectData& project = editorData.project;
			project.name = name;
			project.fileDir = directory + "/" + project.name + "." + projectFileExtension;
			project.editorFileDir = directory + "/" + project.name + "." + projectEditorFileExtension;

			writeFiles(editorData);

			project.isOpen = true;

		}

		void open(EditorData& editorData, std::string name, std::string directory) {
			if (!readFiles(editorData, name, directory)) {
				close(editorData);
				return;
			}

			editorData.project.isOpen = true;
		}
		void open(EditorData& editorData, std::string filepath) {
			if (!readFiles(editorData, filepath)) {
				close(editorData);
				return;
			}
			editorData.project.isOpen = true;
		}

		void close(EditorData& editorData) {
			for (auto& actor : editorData.actors) {
				scene::destroyActor(editorData, 0);
			}
			ProjectData& project = editorData.project;
			project = {};
		}

		void save(EditorData& editorData) {
			for (auto actor : editorData.actors) {
				saveActorFile("Actors", actor, editorData);
			}
			writeFiles(editorData);
		}
	}
}