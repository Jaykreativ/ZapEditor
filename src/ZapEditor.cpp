#include "MainMenuBar.h";
#include "Viewport.h";
#include "SceneHierarchy.h";
#include "ComponentView.h";

#include "Zap/Zap.h"
#include "Zap/ModelLoader.h"
#include "Zap/Event.h"
#include "Zap/EventHandler.h"
#include "Zap/Serializer.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Model.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h";
#include "backends/imgui_impl_glfw.h";
#include "PxPhysicsAPI.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

namespace editor {
	static Zap::Base* engineBase;

	static Zap::Window* window;
	static Zap::Renderer* renderer;

	static Zap::Gui* gui;

	static Zap::PBRenderer* pbr;
	static Zap::RaytracingRenderer* rtx;

	static std::vector<Zap::Scene> scenes;
	
	static uint32_t cam = 0;
	static std::vector<Zap::Actor> actors;
	static std::vector<Zap::Actor> selectedActors;

	static MainMenuBar* mainMenuBar;
	static std::vector<ViewLayer*> layers;

	static Zap::Model cubeModel;
	static int spawnCounter;
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

	auto woodTexture = modelLoader.loadTexture("woodTexture.png");
	auto randomTexture = modelLoader.loadTexture("randomTexture.jpg");
	//auto reddotsTexture = modelLoader.loadTexture("reddotsTexture.jpg");

	editor::cubeModel = modelLoader.load("Models/OBJ/Cube.obj");

	//auto cboxModel = modelLoader.load("Models/gltf/cornellBox.glb");
	
	//auto sponzaModel = modelLoader.load("Models/gltf/Sponza.glb");

	auto gearModel = modelLoader.load("Models/gltf/ZapGear.glb");

	//auto giftModel = modelLoader.load("Models/OBJ/Gift.obj");

	//auto kimberModel = modelLoader.load("Models/OBJ/PistolKimber/PistolKimber.glb");

	auto sphereModel = modelLoader.load("Models/gltf/metalSphere.glb");

	Zap::PhysicsMaterial pxMaterial = Zap::PhysicsMaterial(0.5, 1, 0.1);

	editor::actors.push_back(Zap::Actor());
	auto pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 2.7, 0);
	pActor->addLight({ 12, 12, 12 });

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, -2.7);
	pActor->addLight({ 6, 4, 2 });
	//pActor->addModel(sphereModel);
	//{
	//	Zap::Material mat{};
	//	mat.emissive = {1, 1, 1, 24};
	//	pActor->cmpModel_setMaterial(mat);
	//}

	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scenes.back().attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, -2, 0);
	//pActor->addModel(sponzaModel);

	// coordinate helper
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scene->attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, 5, 0);
	//pActor->cmpTransform_setScale(0.25, 0.25, 0.25);
	//pActor->addModel(cubeModel);
	//{
	//	Zap::Material mat = Zap::Material();
	//	mat.albedoColor = { 1, 1, 1 };
	//	mat.roughness = 0.5;
	//	mat.metallic = 0;
	//	mat.emissive = { 1, 1, 1, 5 };
	//	pActor->cmpModel_setMaterial(mat);
	//}
	//
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scene->attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0.5, 5, 0);
	//pActor->cmpTransform_setScale(0.5, 0.1, 0.1);
	//pActor->addModel(cubeModel);
	//{
	//	Zap::Material mat = Zap::Material();
	//	mat.albedoColor = { 1, 0, 0 };
	//	mat.roughness = 0.5;
	//	mat.metallic = 0;
	//	mat.emissive = { 1, 0, 0, 2 };
	//	pActor->cmpModel_setMaterial(mat);
	//}
	//
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scene->attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, 5.5, 0);
	//pActor->cmpTransform_setScale(0.1, 0.5, 0.1);
	//pActor->addModel(cubeModel);
	//{
	//	Zap::Material mat = Zap::Material();
	//	mat.albedoColor = { 0, 1, 0 };
	//	mat.roughness = 0.5;
	//	mat.metallic = 0;
	//	mat.emissive = { 0, 1, 0, 2 };
	//	pActor->cmpModel_setMaterial(mat);
	//}
	//
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scene->attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, 5, 0.5);
	//pActor->cmpTransform_setScale(0.1, 0.1, 0.5);
	//pActor->addModel(cubeModel);
	//{
	//	Zap::Material mat = Zap::Material();
	//	mat.albedoColor = { 0, 0, 1 };
	//	mat.roughness = 0.5;
	//	mat.metallic = 0;
	//	mat.emissive = { 0, 0, 1, 5 };
	//	pActor->cmpModel_setMaterial(mat);
	//}
	//

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(2, 0, 1);
	pActor->cmpTransform_setScale(0.1, 2, 3);
	pActor->addModel(editor::cubeModel);
	{
		auto mat = Zap::Material();
		mat.setRoughness(0.75);
		mat.setAlbedo({ .75, .25, .25, 1.0});
		pActor->cmpModel_setMaterial(mat);
	}

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-2, 0, 1);
	pActor->cmpTransform_setScale(0.1, 2, 3);
	pActor->addModel(editor::cubeModel);
	{
		auto mat = Zap::Material();
		mat.setRoughness(0.75);
		mat.setAlbedo({ .25, .25, .75, 1.0 });
		pActor->cmpModel_setMaterial(mat);
	}
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, -2);
	pActor->cmpTransform_setScale(2, 2, 0.1);
	pActor->addModel(editor::cubeModel);
	{
		auto mat = Zap::Material();
		mat.setRoughness(0.75);
		mat.setAlbedo({ .75, .75, .75, 1.0 });
		pActor->cmpModel_setMaterial(mat);
	}

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 2, 1);
	pActor->cmpTransform_setScale(2, 0.1, 3);
	pActor->addModel(editor::cubeModel);
	{
		auto mat = Zap::Material();
		mat.setRoughness(0.75);
		mat.setAlbedo({ .75, .75, .75, 1.0 });
		pActor->cmpModel_setMaterial(mat);
	}

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, -2, 1);
	pActor->cmpTransform_setScale(2, 0.1, 3);
	pActor->addModel(editor::cubeModel);
	{
		auto mat = Zap::Material();
		mat.setRoughness(0.75);
		mat.setAlbedo({ .75, .75, .75, 1.0 });
		pActor->cmpModel_setMaterial(mat);
	}

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scenes.back().attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, -1, 1.5);
	pActor->cmpTransform_setScale(0.1, 0.1, 0.1);
	pActor->addModel(gearModel);

	//cboxMat.albedoColor = { .99, .99, .99 };
	//cboxMat.roughness = 0.1;
	//cboxMat.metallic = 1;
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scenes.back().attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0.65, -1.6, 0.8);
	//pActor->cmpTransform_setScale(0.4, 0.4, 0.7);
	//pActor->addModel(sphereModel);
	//pActor->cmpModel_setMaterial(cboxMat);

	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scenes.back().attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(-0.7, -1.2, -0.5);
	//pActor->cmpTransform_setScale(0.8, 0.8, 0.8);
	//pActor->addModel(sphereModel);
	//cboxMat.metallic = 0;
	//pActor->cmpModel_setMaterial(cboxMat);
	
	//for (int x = 0; x < 11; x++) {
	//	for (int z = 0; z < 2; z++) {
	//		editor::actors.push_back(Zap::Actor());
	//		pActor = &editor::actors.back();
	//		editor::scene->attachActor(*pActor);
	//		pActor->addTransform(glm::mat4(1));
	//		pActor->cmpTransform_setPos((x-5)*2, 2, (z-0.5)*3);
	//		pActor->addModel(sphereModel);
	//		Zap::Material material{};
	//		material.albedoColor = {1, 1, 1};
	//		material.roughness = x/10.0;
	//		material.metallic = z;
	//		pActor->cmpModel_setMaterial(material);
	//	}
	//}

	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scenes.back().attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, -6, 0);
	//pActor->cmpTransform_setScale(50, 1, 50);
	//pActor->cmpTransform_rotateX(180);
	//pActor->addModel(cubeModel);
	//pActor->cmpModel_setMaterial(cboxMat);
	//{
	//	Zap::Shape shape(Zap::BoxGeometry({50, 1, 50}), pxMaterial, true);
	//	pActor->addRigidStatic(shape);
	//}

	//for (int i = 0; i < 5; i++) {
	//	for (int j = 0; j < 5; j++) {
	//		for (int k = 0; k < 5-j; k++) {
	//			editor::actors.push_back(Zap::Actor());
	//			pActor = &editor::actors.back();
	//			editor::scenes.back().attachActor(*pActor);
	//			pActor->addTransform(glm::mat4(1));
	//			pActor->cmpTransform_setPos(i+5, 0.3+j*0.6, k * 0.6 + j * 0.6 / 2.0);
	//			pActor->cmpTransform_setScale(0.3, 0.3, 0.3);
	//			pActor->addModel(giftModel);
	//			Zap::Material material{};
	//			material.albedoColor = {sin(i*2), 1, sin(i*5)};
	//			material.albedoMap = j%2;
	//			material.roughness = std::min<float>(j/5.0+0.1, 1);
	//			material.metallic = !k%2;
	//			pActor->cmpModel_setMaterial(material);
	//			{
	//				Zap::Shape shape(Zap::BoxGeometry({ 0.3, 0.3, 0.3 }), pxMaterial, true);
	//				pActor->addRigidDynamic(shape);
	//			}
	//		}
	//	}
	//}

	// Kimber Pistol Cube Generator
	//glm::vec3 kpcPos = { 0, 1, 0 };
	//float kpcPadding = 0.25;
	//glm::vec3 kpcSize = { 10, 10, 10 };
	//glm::vec3 kpcCorner = kpcPos - kpcSize * 0.5f * kpcPadding;
	//for (float x = 0; x < kpcSize.x*kpcPadding; x += kpcPadding) {
	//	for (float y = 0; y < kpcSize.y * kpcPadding; y += kpcPadding) {
	//		for (float z = 0; z < kpcSize.z * kpcPadding; z += kpcPadding) {
	//			editor::actors.push_back(Zap::Actor());
	//			pActor = &editor::actors.back();
	//			editor::scenes.back().attachActor(*pActor);
	//			pActor->addTransform(glm::mat4(1));
	//			pActor->cmpTransform_setPos(kpcCorner + glm::vec3(x, y, z));
	//			pActor->cmpTransform_setScale(1);
	//			pActor->addModel(kimberModel);
	//		}
	//	}
	//}

}

void windowResizeCallback(Zap::ResizeEvent& params, void* data) {}

int main() {
	editor::engineBase = Zap::Base::createBase("Zap Application");
	auto settings = editor::engineBase->getSettings();
	
	//std::cout << "Enable raytracing 1(true) | 0(false)\n>>> ";
	//std::cin >> settings->enableRaytracing;
	settings->enableRaytracing = true;

	editor::engineBase->init();

	editor::window = new Zap::Window(1000, 600, "Zap Window");
	editor::window->init();
	editor::window->getResizeEventHandler()->addCallback(windowResizeCallback);

	editor::renderer = new Zap::Renderer();

	Zap::Gui::initImGui(editor::window);
	editor::gui = new Zap::Gui();

	//deserialize

	//Zap::Serializer deserializer;
	//deserializer.deserialize("./Actors", &editor::actors, &editor::scenes);
	editor::scenes.push_back(Zap::Scene());
	editor::scenes.back().init();
	
	setupActors();

	for(auto& scene : editor::scenes)
		scene.update();

	editor::renderer->setTarget(editor::window);

	editor::renderer->addRenderTask(editor::gui);

	editor::renderer->init();
	editor::renderer->beginRecord();
	editor::renderer->recRenderTemplate(editor::gui);
	editor::renderer->endRecord();

	editor::mainMenuBar = new editor::MainMenuBar(editor::layers, editor::window, editor::renderer, editor::gui, &editor::scenes.back(), editor::actors, editor::selectedActors);
	editor::layers.push_back(new editor::Viewport(&editor::scenes.back(), editor::window, editor::selectedActors));
	editor::layers.push_back(new editor::SceneHierarchyView(&editor::scenes.back(), editor::actors, editor::selectedActors));
	editor::layers.push_back(new editor::ComponentView(editor::selectedActors));

	setupGuiStyle();

	editor::window->show();

	Zap::PhysicsMaterial pxMaterial = Zap::PhysicsMaterial(0.5, 1, 0.1);
	//mainloop
	float dTime = 0;
	uint64_t frameIndex = 0;
	while (!editor::window->shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		
		//editor::actors[3].cmpTransform_rotateY(15*dTime);

		if (!editor::window->isIconified()) {
			ImGui::DockSpaceOverViewport(0U, ImGui::GetMainViewport());

			ImGui::ShowDemoWindow();

			if (ImGui::Button("Spawn")) {
				editor::actors.push_back(Zap::Actor());
				auto* pActor = &editor::actors.back();
				editor::scenes.back().attachActor(*pActor);
				pActor->addTransform(glm::mat4(1));
				pActor->cmpTransform_setPos(1.0f+editor::spawnCounter*2.5f, 2.6f, 2.0f);
				pActor->addModel(editor::cubeModel);
				editor::spawnCounter++;
			}

			editor::mainMenuBar->draw();

			uint32_t i = 0;
			for (auto layer : editor::layers) {
				ImGui::Begin((layer->name() + std::string("###") + std::to_string(i + 1)).c_str(), nullptr, layer->getWindowFlags() | ImGuiWindowFlags_MenuBar);

				ImGui::BeginMenuBar();
				bool isClosed = false;
				if (ImGui::BeginMenu("View")) {
					if (ImGui::MenuItem("Close")) {
						delete layer;
						editor::layers.erase(editor::layers.begin()+i);
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

		if (editor::mainMenuBar->shouldSimulate() && dTime > 0) {
			for(auto& scene : editor::scenes)
				scene.simulate(dTime);
		}

		// render GUI only
		editor::renderer->render();

		editor::window->present();
		Zap::Window::pollEvents();
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		frameIndex++;
	}
	//serialize
	
	Zap::Serializer serializer;
	for (auto actor : editor::actors) {
		serializer.addActor(actor);
	}
	//serializer.serialize("./Actors");

	//terminate
	editor::renderer->destroy();

	for (auto layer : editor::layers) {
		delete layer;
	}
	editor::layers.clear();

	delete editor::renderer;

	Zap::Gui::destroyImGui();

	delete editor::window;

	for(auto scene : editor::scenes)
		scene.destroy();
	editor::scenes.clear();

	delete editor::gui;
	delete editor::pbr;

	editor::engineBase->terminate();
	Zap::Base::releaseBase();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}