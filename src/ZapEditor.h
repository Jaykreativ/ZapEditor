#pragma once

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

	class SceneHandler;
	class ViewLayer;
	struct EditorData {
		std::unique_ptr<SceneHandler> pSceneHandler;

		std::vector<Zap::Shape>           physicsShapes    = {};
		std::vector<Zap::PhysicsMaterial> physicsMaterials = {};
		std::vector<Zap::ConvexMesh>      convexMeshes     = {};

		std::vector<Zap::Actor> selectedActors   = {};

		struct DefaultMeshes {
			DefaultMeshes(Zap::AssetHandle<Zap::Mesh> cube, Zap::AssetHandle<Zap::Mesh> sphere, Zap::AssetHandle<Zap::Mesh> cylinder, Zap::AssetHandle<Zap::Mesh> cone, Zap::AssetHandle<Zap::Mesh> torus)
				: cube(cube), sphere(sphere), cylinder(cylinder), cone(cone), torus(torus)
			{}
			Zap::AssetHandle<Zap::Mesh> cube;
			Zap::AssetHandle<Zap::Mesh> sphere;
			Zap::AssetHandle<Zap::Mesh> cylinder;
			Zap::AssetHandle<Zap::Mesh> cone;
			Zap::AssetHandle<Zap::Mesh> torus;
		};
		std::unique_ptr<DefaultMeshes> pDefaultMeshes;
		Zap::AssetHandle<Zap::Material> defaultMaterial;

		Zap::Base* engineBase;

		Zap::Window* window;
		Zap::Renderer* renderer;

		Zap::RenderTaskHandle<Zap::Gui> guiTask;

		std::vector<ViewLayer*> layers;

		ProjectData project = {};

		float dTime = 0;
	};
}
