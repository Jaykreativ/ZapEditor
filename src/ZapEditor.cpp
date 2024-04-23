#include "MainMenuBar.h";
#include "Viewport.h";
#include "SceneHierarchy.h";
#include "ComponentView.h";

#include "Zap/Zap.h"
#include "Zap/ModelLoader.h"
#include "Zap/EventHandler.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/RaytracingRenderer.h"
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

	static Zap::Scene* scene;

	static Zap::EventHandler* eventHandler;
	
	static uint32_t cam = 0;
	static std::vector<Zap::Actor> actors;
	static std::vector<Zap::Actor> selectedActors;

	static MainMenuBar* mainMenuBar;
	static std::vector<ViewLayer*> layers;
}

void resize(GLFWwindow* window, int width, int height) {}

void setupActors() {
	Zap::ModelLoader modelLoader = Zap::ModelLoader();

	auto woodTexture = modelLoader.loadTexture("woodTexture.png");
	//auto randomTexture = modelLoader.loadTexture("randomTexture.jpg");
	//auto reddotsTexture = modelLoader.loadTexture("reddotsTexture.jpg");

	auto cubeModel = modelLoader.load("Models/OBJ/Cube.obj");
	//auto cboxModel = modelLoader.load("Models/gltf/cornellBox.glb");
	//
#ifndef _DEBUG
	//auto sponzaModel = modelLoader.load("Models/OBJ/Sponza/Sponza.obj");
#endif

	//auto giftModel = modelLoader.load("Models/OBJ/Gift.obj");

	//auto kimberModel = modelLoader.load("Models/OBJ/PistolKimber/PistolKimber.glb");

	auto sphereModel = modelLoader.load("Models/gltf/metalSphere.glb");

	Zap::PhysicsMaterial pxMaterial = Zap::PhysicsMaterial(0.5, 1, 0.1);

	editor::actors.push_back(Zap::Actor());
	auto pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 2.7, 0);
	pActor->addLight({ 12, 12, 12 });

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, -2.7);
	pActor->addLight({ 6, 4, 2 });
	//pActor->addModel(sphereModel);
	//{
	//	Zap::Material mat{};
	//	mat.emissive = {1, 1, 1, 24};
	//	pActor->cmpModel_setMaterial(mat);
	//}

#ifndef _DEBUG
	//editor::actors.push_back(Zap::Actor());
	//pActor = &editor::actors.back();
	//editor::scene->attachActor(*pActor);
	//pActor->addTransform(glm::mat4(1));
	//pActor->cmpTransform_setPos(0, 0, 0);
	//pActor->cmpTransform_setScale(0.02, 0.02, 0.02);
	//pActor->addModel(sponzaModel);
#endif

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

	Zap::Material cboxMat{};
	cboxMat.roughness = 1;

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(2, 0, 1);
	pActor->cmpTransform_setScale(0.1, 2, 3);
	pActor->addModel(cubeModel);
	cboxMat.albedoColor = { .75, .25, .25 };
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-2, 0, 1);
	pActor->cmpTransform_setScale(0.1, 2, 3);
	pActor->addModel(cubeModel);
	cboxMat.albedoColor = { .25, .25, .75 };
	pActor->cmpModel_setMaterial(cboxMat);
	cboxMat.albedoColor = { .75, .75, .75 };

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, -2);
	pActor->cmpTransform_setScale(2, 2, 0.1);
	pActor->addModel(cubeModel);
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 2, 1);
	pActor->cmpTransform_setScale(2, 0.1, 3);
	pActor->addModel(cubeModel);
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, -2, 1);
	pActor->cmpTransform_setScale(2, 0.1, 3);
	pActor->addModel(cubeModel);
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0.75, -1.2, -0.8);
	pActor->cmpTransform_setScale(0.8, 0.8, 0.8);
	pActor->addModel(sphereModel);
	cboxMat.albedoColor = { .99, .99, .99 };
	cboxMat.roughness = 0.1;
	cboxMat.metallic = 1;
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0.65, -1.6, 0.8);
	pActor->cmpTransform_setScale(0.4, 0.4, 0.7);
	pActor->addModel(sphereModel);
	pActor->cmpModel_setMaterial(cboxMat);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-0.7, -0.9, 0.7);
	pActor->cmpTransform_setScale(0.8, 0.8, 0.8);
	pActor->addModel(sphereModel);
	cboxMat.metallic = 0;
	pActor->cmpModel_setMaterial(cboxMat);

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

	//for (int i = 0; i < 5; i++) {
	//	for (int j = 0; j < 5; j++) {
	//		for (int k = 0; k < 5-j; k++) {
	//			editor::actors.push_back(Zap::Actor());
	//			pActor = &editor::actors.back();
	//			editor::scene->attachActor(*pActor);
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
}

int main() {

	//class A {
	//public:
	//	A() {}
	//	virtual ~A() {}
	//
	//	virtual void init() {
	//		std::cout << "init: A\n";
	//	}
	//};
	//
	//class B : public A {
	//public:
	//	B() {}
	//	~B() {}
	//
	//	void init() {
	//		A::init();
	//		std::cout << "init: B\n";
	//	}
	//};
	//
	//A* obj = new B();
	//obj->init();
	//delete obj;
	//
	//system("pause");

	editor::engineBase = Zap::Base::createBase("Zap Application");
	auto settings = editor::engineBase->getSettings();
	
	std::cout << "Enable raytracing 1(true) | 0(false)\n>>> ";
	std::cin >> settings->enableRaytracing;

	editor::engineBase->init();

	editor::scene = new Zap::Scene();
	editor::scene->init();

	editor::window = new Zap::Window(1000, 600, "Zap Window");
	editor::window->init();
	editor::window->show();
	//editor::window->setMousebButtonCallback(keybinds::mouseButtonCallback);
	//editor::window->setKeyCallback(keybinds::keyCallback);
	editor::window->setResizeCallback(resize);

	editor::eventHandler = new Zap::EventHandler(editor::window);

	editor::renderer = new Zap::Renderer(*editor::window);

	editor::gui = new Zap::Gui(*editor::renderer);

	setupActors();

	editor::scene->update();

	editor::gui->init();

	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_Header] = ImVec4(100 /255.0, 95 /255.0, 90 /255.0, 80 /255.0);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(125 /255.0, 120 /255.0, 115 /255.0, 200 /255.0);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(155 /255.0, 150 /255.0, 145 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_Separator] = ImVec4(125 /255.0, 115 /255.0, 110 /255.0, 125 /255.0);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(40 /255.0, 37 /255.0, 35 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(50 /255.0, 47 /255.0, 45 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(55 /255.0, 52 /255.0, 50 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_Tab] = ImVec4(155 /255.0, 85 /255.0, 45 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(250 /255.0, 145 /255.0, 85 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TabActive] = ImVec4(225 /255.0, 125 /255.0, 75 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(75 /255.0, 72 /255.0, 70 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(115 /255.0, 112 /255.0, 110 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(185 /255.0, 95 /255.0, 45 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(200 /255.0, 105 /255.0, 55 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(250 /255.0, 135 /255.0, 75 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(55 /255.0, 52 /255.0, 50 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(85 /255.0, 82 /255.0, 80 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(175 /255.0, 100 /255.0, 55 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(230 /255.0, 135 /255.0, 100 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(185 /255.0, 105 /255.0, 65 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(230 /255.0, 135 /255.0, 100 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_Button] = ImVec4(65 /255.0, 62 /255.0, 60 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(105 /255.0, 102 /255.0, 100 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(200 /255.0, 115 /255.0, 65 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_DockingPreview] = ImVec4(255 /255.0, 145 /255.0, 85 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(190 /255.0, 100 /255.0, 55 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(200 /255.0, 105 /255.0, 60 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(250 /255.0, 135 /255.0, 85 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_NavHighlight] = ImVec4(250 /255.0, 135 /255.0, 85 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(50 /255.0, 48 /255.0, 46 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(85 /255.0, 75 /255.0, 72 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_TableBorderLight] = ImVec4(65 /255.0, 62 /255.0, 60 /255.0, 255 /255.0);
	style->Colors[ImGuiCol_Border] = ImVec4(130 /255.0, 112 /255.0, 110 /255.0, 255 /255.0);

	editor::renderer->addRenderTemplate(editor::gui);

	editor::mainMenuBar = new editor::MainMenuBar(editor::layers, editor::renderer, editor::scene, editor::eventHandler, editor::actors, editor::selectedActors);
	editor::layers.push_back(new editor::Viewport(editor::renderer, editor::scene, editor::eventHandler));
	editor::layers.push_back(new editor::SceneHierarchyView(editor::actors, editor::selectedActors));
	editor::layers.push_back(new editor::ComponentView(editor::selectedActors));

	editor::renderer->init();

	//mainloop
	float dTime = 0;
	uint64_t frameIndex = 0;
	while (!editor::window->shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		
		//editor::actors[3].cmpTransform_rotateY(15*dTime);

		if (!editor::window->isIconified()) {
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

			ImGui::ShowDemoWindow();

			editor::mainMenuBar->draw();

			if (ImGui::Button("Spawn")) {
				editor::actors.push_back(Zap::Actor());
				auto* pActor = &editor::actors.back();
				editor::scene->attachActor(*pActor);
				pActor->addTransform(glm::mat4(1));
				pActor->cmpTransform_setPos(0, 2.5, 0);
				pActor->addLight({ 1, 1, 1 }, 5);
			}
			if (ImGui::Button("Delete")) {
				editor::actors.back().destroy();
				editor::actors.pop_back();
			}

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
			editor::scene->simulate(dTime);
		}

		editor::scene->update();
		editor::renderer->render();

		Zap::Window::pollEvents();
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		frameIndex++;
	}

	//terminate
	editor::renderer->destroy();

	for (auto layer : editor::layers) {
		delete layer;
	}
	editor::layers.clear();

	delete editor::renderer;
	delete editor::window;

	editor::scene->destroy();
	delete editor::scene;

	delete editor::gui;
	delete editor::pbr;

	editor::engineBase->terminate();
	Zap::Base::releaseBase();

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}