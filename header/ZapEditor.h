#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"

#include <string>

namespace editor {
	struct ProjectData {
		bool isOpen = false;
		std::string name = "";
		std::string fileDir = "";
		std::string editorFileDir = "";

		std::unordered_map<Zap::UUID, std::string> actorPathMap = {};
		std::unordered_map<Zap::UUID, std::string> scenePathMap = {};
	};

	struct EditorData {
		std::vector<Zap::Scene> scenes = {};
		std::vector<Zap::Actor> actors = {};

		std::unordered_map<Zap::UUID, std::string> actorNameMap     = {};
		std::vector<Zap::Shape>                    physicsShapes    = {};
		std::vector<Zap::PhysicsMaterial>          physicsMaterials = {};
		std::vector<Zap::Actor>                    selectedActors   = {};

		ProjectData project = {};
	};
}
