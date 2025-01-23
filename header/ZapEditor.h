#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"

#include <string>

namespace editor {
	static const std::string actorFileExtension = "zac";
	static const std::string sceneFileExtension = "zsc";
	static const std::string projectFileExtension = "zproj";
	static const std::string projectEditorFileExtension = "zproj.edit";

	struct ProjectData {
		bool isOpen = false;
		std::string name = "";
		std::string fileDir = "";
		std::string editorFileDir = "";
	};

	struct EditorData {
		std::vector<Zap::Scene> scenes = {};
		std::vector<Zap::Actor> actors = {};

		// custom ECS components
		std::unordered_map<Zap::UUID, std::string> actorNameMap     = {};
		std::unordered_map<Zap::UUID, std::string> actorPathMap = {};
		std::unordered_map<Zap::UUID, std::string> scenePathMap = {};

		std::vector<Zap::Shape>                    physicsShapes    = {};
		std::vector<Zap::PhysicsMaterial>          physicsMaterials = {};
		std::vector<Zap::Actor>                    selectedActors   = {};

		ProjectData project = {};
	};
}
