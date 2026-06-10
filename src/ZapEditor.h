#pragma once

#include "ViewLayer.h"

#include "Zap/UUID.h"
#include "Zap/Physics/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/Gui.h"

#include <string>
#include <filesystem>

namespace editor {
	static const std::string actorFileExtension = "zac";
	static const std::string sceneFileExtension = "zsc";
	static const std::string assetLibraryFileExtension = "zal";
	static const std::string projectFileExtension = "zproj";
	static const std::string projectEditorFileExtension = "zproj.edit";

	struct ProjectData {
		bool isOpen = false;
		std::filesystem::path rootPath = ""; // the directory where the project file is located

		std::filesystem::path projectFile = ""; // path to the .zproj file
		std::filesystem::path editorFile = ""; // path to the .zproj.edit file

		// saved data
		std::string name = "";
		std::filesystem::path assetLibraryPath = "";
	};

	struct EditorData {
		std::vector<Zap::Scene> scenes = {};
		std::vector<Zap::Actor> actors = {};

		// custom ECS components
		std::unordered_map<Zap::UUID, std::string> actorNameMap = {};
		std::unordered_map<Zap::UUID, std::filesystem::path> actorPathMap = {};
		std::unordered_map<Zap::UUID, std::filesystem::path> scenePathMap = {};

		std::vector<Zap::Shape>           physicsShapes    = {};
		std::vector<Zap::PhysicsMaterial> physicsMaterials = {};
		std::vector<Zap::ConvexMesh>      convexMeshes     = {};

		std::vector<Zap::Actor> selectedActors   = {};
		Zap::Scene*             pActiveScene = nullptr;

		struct DefaultMeshes {
			DefaultMeshes(Zap::Mesh cube, Zap::Mesh sphere, Zap::Mesh cylinder, Zap::Mesh cone, Zap::Mesh torus)
				: cube(cube), sphere(sphere), cylinder(cylinder), cone(cone), torus(torus)
			{}
			Zap::Mesh cube;
			Zap::Mesh sphere;
			Zap::Mesh cylinder;
			Zap::Mesh cone;
			Zap::Mesh torus;
		};
		std::unique_ptr<DefaultMeshes> pDefaultMeshes;

		Zap::Base* engineBase;

		Zap::Window* window;
		Zap::Renderer* renderer;

		Zap::RenderTaskHandle<Zap::Gui> guiTask;

		std::vector<ViewLayer*> layers;

		ProjectData project = {};

		float dTime = 0;
	};
}
