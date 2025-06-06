#include "MainMenuBar.h";
#include "Viewport.h";
#include "SceneHierarchy.h";
#include "ComponentView.h";
#include "Settings.h";
#include "FileHandling.h"
#include "SceneHandling.h"

#include "Zap/Zap.h"
#include "Zap/FileLoader.h"
#include "Zap/Event.h"
#include "Zap/EventHandler.h"
#include "Zap/Serializer.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Model.h"
#include "Zap/Physics/Shape.h"
#include "Zap/Physics/HitMesh.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h";
#include "backends/imgui_impl_glfw.h";
#include "ZapEditor.h"

#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <filesystem>

namespace editor {
	static EditorData editorData = {};
	
	static uint32_t cam = 0;

	static MainMenuBar* mainMenuBar;

	static Zap::Model cubeModel;
}

void setupGuiStyle() {
	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_Header] = ImVec4(100 / 255.0, 95 / 255.0, 90 / 255.0, 80 / 255.0);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(125 / 255.0, 120 / 255.0, 115 / 255.0, 200 / 255.0);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(155 / 255.0, 150 / 255.0, 145 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_Separator] = ImVec4(125 / 255.0, 115 / 255.0, 110 / 255.0, 125 / 255.0);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(40 / 255.0, 37 / 255.0, 35 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(50 / 255.0, 47 / 255.0, 45 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(55 / 255.0, 52 / 255.0, 50 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_Tab] = ImVec4(155 / 255.0, 85 / 255.0, 45 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(250 / 255.0, 145 / 255.0, 85 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TabActive] = ImVec4(225 / 255.0, 125 / 255.0, 75 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(75 / 255.0, 72 / 255.0, 70 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(115 / 255.0, 112 / 255.0, 110 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(185 / 255.0, 95 / 255.0, 45 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(200 / 255.0, 105 / 255.0, 55 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(250 / 255.0, 135 / 255.0, 75 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(55 / 255.0, 52 / 255.0, 50 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(85 / 255.0, 82 / 255.0, 80 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(175 / 255.0, 100 / 255.0, 55 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(230 / 255.0, 135 / 255.0, 100 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(185 / 255.0, 105 / 255.0, 65 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(230 / 255.0, 135 / 255.0, 100 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_Button] = ImVec4(65 / 255.0, 62 / 255.0, 60 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(105 / 255.0, 102 / 255.0, 100 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(230 / 255.0, 100 / 255.0, 60 / 255.0, 1);
	style->Colors[ImGuiCol_DockingPreview] = ImVec4(255 / 255.0, 145 / 255.0, 85 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(190 / 255.0, 100 / 255.0, 55 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(200 / 255.0, 105 / 255.0, 60 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(250 / 255.0, 135 / 255.0, 85 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_NavHighlight] = ImVec4(250 / 255.0, 135 / 255.0, 85 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(50 / 255.0, 48 / 255.0, 46 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(85 / 255.0, 75 / 255.0, 72 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_TableBorderLight] = ImVec4(65 / 255.0, 62 / 255.0, 60 / 255.0, 255 / 255.0);
	style->Colors[ImGuiCol_Border] = ImVec4(130 / 255.0, 112 / 255.0, 110 / 255.0, 255 / 255.0);
}

void setupActors() {
	Zap::ModelLoader modelLoader = Zap::ModelLoader();
	Zap::ActorLoader actorLoader = Zap::ActorLoader();
	Zap::HitMeshLoader hitMeshLoader = Zap::HitMeshLoader();

	editor::cubeModel = modelLoader.load((std::string)"Models/OBJ/Cube.obj");

	auto cubeHitMesh = hitMeshLoader.load("Models/OBJ/Cube.obj");

	//auto cboxModel = modelLoader.load("Models/gltf/cornellBox.glb");
	
	//auto sponzaModel = modelLoader.load("Models/OBJ/Sponza/Sponza.obj");
	//auto sponzaModel = modelLoader.load("Models/gltf/Sponza.glb");

	//auto gearModel = modelLoader.load("Models/gltf/ZapGear.glb");

	//auto sphereModel = modelLoader.load("Models/gltf/metalSphere.glb");

	//auto pistolKimber = modelLoader.load((std::string)"Models/gltf/PistolKimber/PistolKimber.glb");

	//glm::u8vec4 texCol = { 255, 180, 50, 255 };
	//modelLoader.loadTexture(&texCol, 1, 1);

	Zap::PhysicsMaterial pxMaterial = Zap::PhysicsMaterial(0.5, 1, 0.1);

	editor::editorData.actors.push_back(Zap::Actor());
	auto pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "LightWhite";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-2, 2.7, 1);
	pActor->addLight({ 1, 1, 1 }, 20);

	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "LightOrange";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(2, 2, 2.7);
	pActor->addLight({ 1, .6, .2 }, 10);

	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "Cube";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, 5);
	pActor->addModel(editor::cubeModel);
	{
		Zap::ConvexMesh convexMesh(cubeHitMesh);
		editor::editorData.convexMeshes.push_back(convexMesh);
		auto geometry = Zap::ConvexMeshGeometry(convexMesh);
		Zap::Shape shape(geometry, pxMaterial, true);
		pActor->addRigidDynamic(shape);
	}

	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scenes.back().attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, -2, 0);
	//pActor->cmpTransform_setScale(0.01);
	//pActor->addModel(sponzaModel);

	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "Ground";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, -6, 0);
	pActor->cmpTransform_setScale(50, 1, 50);
	pActor->cmpTransform_rotateX(180);
	pActor->addModel(editor::cubeModel);
	{
		auto geometry = Zap::BoxGeometry(glm::vec3(50, 1, 50));
		Zap::Shape shape(geometry, pxMaterial, true);
		pActor->addRigidStatic(shape);
	}

	// coordinate helper
	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "Zero";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, 0);
	pActor->cmpTransform_setScale(0.25, 0.25, 0.25);
	pActor->addModel(editor::cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.setEmissive({ 1, 1, 1, 5 });
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "X";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0.5, 0, 0);
	pActor->cmpTransform_setScale(0.5, 0.1, 0.1);
	pActor->addModel(editor::cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.setEmissive({ 1, 0, 0, 2 });
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "Y";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0.5, 0);
	pActor->cmpTransform_setScale(0.1, 0.5, 0.1);
	pActor->addModel(editor::cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.setEmissive({ 0, 1, 0, 2 });
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::editorData.actors.push_back(Zap::Actor());
	pActor = &editor::editorData.actors.back();
	editor::editorData.scenes.back().attachActor(*pActor);
	editor::editorData.actorNameMap[*pActor] = "Z";
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, 0.5);
	pActor->cmpTransform_setScale(0.1, 0.1, 0.5);
	pActor->addModel(editor::cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.setEmissive({ 0, 0, 1, 5 });
		pActor->cmpModel_setMaterial(mat);
	}
}

void windowResizeCallback(Zap::ResizeEvent& params, void* data) {}

void dragDropCallback(Zap::DragDropEvent& params, void* customData) {
	for (int i = 0; i < params.pathCount; i++) {
		loadFile(params.paths[i], editor::editorData);
	}
}

int main() {
	editor::editorData.engineBase = Zap::Base::createBase("Zap Application"); // Don't automatically load/save AssetLibrary
	auto settings = editor::editorData.engineBase->getSettings();
	
	//std::cout << "Enable raytracing 1(true) | 0(false)\n>>> ";
	//std::cin >> settings->enableRaytracing;
	settings->enableRaytracing = true;

	editor::editorData.engineBase->init();

	editor::editorData.window = new Zap::Window(1000, 600, "Zap Window");
	editor::editorData.window->init();
	editor::editorData.window->getResizeEventHandler()->addCallback(windowResizeCallback);
	editor::editorData.window->getDragDropEventHandler()->addCallback(dragDropCallback);

#ifdef _DEBUG
	// compile shaders
	vk::Shader::compile("../Zap/Shader/src/",{
		"PBRShader.vert", "PBRShader.frag",
		"pathTrace.rchit", "pathTrace.rgen", "pathTrace.rint", "pathTrace.rmiss",
		"raytrace.rchit", "raytrace.rgen", "raytrace.rmiss", "raytraceShadow.rmiss",
		}, { "./" });
	vk::Shader::compile("./Shader/src/", {
		"debug.vert", "debug.frag",
		"outline.frag", "outlineBlur.vert", "outlineBlurA.frag", "outlineBlurB.frag", "outlinePlain.frag", "outlinePlain.vert"
		}, {"./"});
#endif

	editor::editorData.renderer = new Zap::Renderer();

	Zap::Gui::initImGui(editor::editorData.window);
	editor::editorData.gui = new Zap::Gui();

	//deserialize

	auto& scene = editor::scene::createScene(editor::editorData);
	scene.init();
	editor::scene::selectScene(editor::editorData, scene);
	
	//setupActors();

	for(auto& scene : editor::editorData.scenes)
		scene.update();

	editor::editorData.renderer->setTarget(editor::editorData.window);

	editor::editorData.renderer->addRenderTask(editor::editorData.gui);

	editor::editorData.renderer->init();
	editor::editorData.renderer->beginRecord();
	editor::editorData.renderer->recRenderTemplate(editor::editorData.gui);
	editor::editorData.renderer->endRecord();

	editor::mainMenuBar = new editor::MainMenuBar(&editor::editorData, editor::editorData.layers, editor::editorData.window, editor::editorData.renderer, editor::editorData.gui, &editor::editorData.scenes.back(), editor::editorData.actors, editor::editorData.selectedActors);
	editor::editorData.layers.push_back(new editor::Viewport(editor::editorData, &editor::editorData.scenes.back(), editor::editorData.window));
	editor::editorData.layers.push_back(new editor::SceneHierarchyView(&editor::editorData, &editor::editorData.scenes.back()));
	editor::editorData.layers.push_back(new editor::ComponentView(&editor::editorData, editor::editorData.layers, editor::editorData.selectedActors));
	editor::editorData.layers.push_back(new editor::Settings(&editor::editorData));

	setupGuiStyle();

	editor::editorData.window->show();

	editor::editorData.physicsMaterials.push_back(Zap::PhysicsMaterial(0.5, 1, 0.1));
	//mainloop
	uint64_t frameIndex = 0;
	while (!editor::editorData.window->shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();

		if (!editor::editorData.window->isIconified()) {
			ImGui::DockSpaceOverViewport(0U, ImGui::GetMainViewport());

			ImGui::ShowDemoWindow();

			// for quick string modification testing
			//static char cbuf0[250] = {};
			//static char cbuf1[250] = {};
			//ImGui::InputText("path", cbuf0, 250);
			//std::filesystem::path path = cbuf0;
			//ImGui::InputText("base", cbuf1, 250);
			//std::filesystem::path base = cbuf1;
			//
			//ImGui::Text(processPath(path, base).string().c_str());

			editor::mainMenuBar->draw();

			uint32_t i = 0;
			for (auto layer : editor::editorData.layers) {
				ImGui::Begin((layer->name() + std::string("###") + std::to_string(i + 1)).c_str(), nullptr, layer->getWindowFlags() | ImGuiWindowFlags_MenuBar);

				ImGui::BeginMenuBar();
				bool isClosed = false;
				if (ImGui::BeginMenu("View")) {
					if (ImGui::MenuItem("Close")) {
						delete layer;
						editor::editorData.layers.erase(editor::editorData.layers.begin()+i);
						isClosed = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();

				if(!isClosed)
					layer->draw();

				ImGui::End();
				i++;
			}
		}

		if (editor::mainMenuBar->shouldSimulate() && editor::editorData.dTime > 0) {
			for(auto& scene : editor::editorData.scenes)
				scene.simulate(editor::editorData.dTime);
		}

		// render GUI only
		editor::editorData.renderer->render();

		editor::editorData.window->present();
		Zap::Window::pollEvents();
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		editor::editorData.dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		frameIndex++;
	}
	//serialize
	//actorLoader.store("Actors/cube.zac", editor::actors[0]);
	//actorLoader.store("Actors/ground.zac", editor::actors[1]);
	//actorLoader.store("Actors/light.zac", editor::actors[2]);
	//actorLoader.store("Actors/lightOrange.zac", editor::actors[3]);
	
	//terminate
	editor::editorData.renderer->destroy();

	for (auto layer : editor::editorData.layers) {
		delete layer;
	}
	editor::editorData.layers.clear();

	for (auto convexMesh : editor::editorData.convexMeshes)
		convexMesh.release();
	for (auto physicsMaterial : editor::editorData.physicsMaterials)
		physicsMaterial.release();

	delete editor::editorData.renderer;

	Zap::Gui::destroyImGui();

	delete editor::editorData.window;

	for(auto scene : editor::editorData.scenes)
		scene.destroy();
	editor::editorData.scenes.clear();
	editor::editorData.actors.clear();

	delete editor::editorData.gui;

	editor::editorData.engineBase->terminate();
	Zap::Base::releaseBase();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}