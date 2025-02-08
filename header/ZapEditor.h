#pragma once

#include "ViewLayer.h"

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/Window.h"

#include <string>

namespace editor {
	static const std::string actorFileExtension = "zac";
	static const std::string sceneFileExtension = "zsc";
	static const std::string assetLibraryFileExtension = "zal";
	static const std::string projectFileExtension = "zproj";
	static const std::string projectEditorFileExtension = "zproj.edit";

	struct ProjectData {
		bool isOpen = false;
		std::string rootPath = ""; // the directory where the project file is located
		std::string fileDir = "";
		std::string editorFileDir = "";

		// saved data
		std::string name = "";
		std::string assetLibraryPath = "";
	};

	struct EditorData {
		std::vector<Zap::Scene> scenes = {};
		std::vector<Zap::Actor> actors = {};

		// custom ECS components
		std::unordered_map<Zap::UUID, std::string> actorNameMap = {};
		std::unordered_map<Zap::UUID, std::string> actorPathMap = {};
		std::unordered_map<Zap::UUID, std::string> scenePathMap = {};

		std::vector<Zap::Shape>           physicsShapes    = {};
		std::vector<Zap::PhysicsMaterial> physicsMaterials = {};

		std::vector<Zap::Actor> selectedActors   = {};
		Zap::Scene*             pActiveScene = nullptr;

		Zap::Base* engineBase;

		Zap::Window* window;
		Zap::Renderer* renderer;

		Zap::Gui* gui;

		std::vector<ViewLayer*> layers;

		ProjectData project = {};
	};
}
