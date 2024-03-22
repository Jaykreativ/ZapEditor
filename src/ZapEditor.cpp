#include "MainMenuBar.h";
#include "Viewport.h";
#include "SceneHierarchy.h";
#include "ComponentView.h";

#include "Zap/Zap.h"
#include "Zap/ModelLoader.h"
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
	
	static uint32_t cam = 0;
	static std::vector<Zap::Actor> actors;

	static MainMenuBar* mainMenuBar;
	static Viewport* viewport;
	static SceneHierarchyView* sceneHierarchyView;
	static ComponentView* componentView;
}

namespace movement {
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool up = false;
	bool turnCamera = false;
	void move(float dTime) {
		if (forward) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			glm::vec3 vec = res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
		if (backward) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			glm::vec3 vec = -res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
		if (right) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			glm::vec3 vec = res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
		if (left) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			glm::vec3 vec = -res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
		if (down) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
		if (up) {
			auto res = editor::actors[editor::cam].cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			editor::actors[editor::cam].cmpTransform_setTransform(res);
		}
	}

	double xlast = 0;
	double ylast = 0;
	float sensitivityX = 0.2;
	float sensitivityY = 0.15;
	void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		if (turnCamera) {		
			glm::mat4 res = editor::actors[editor::cam].cmpTransform_getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>((xpos-xlast)*sensitivityX), glm::vec3{ 0, 1, 0 });

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			editor::actors[editor::cam].cmpTransform_setTransform(res);
			editor::actors[editor::cam].cmpTransform_rotateX((ypos-ylast)*sensitivityY);
		}

		xlast = xpos;
		ylast = ypos;
	}
}

namespace keybinds {
	int forward = GLFW_KEY_W;
	int backward = GLFW_KEY_S;
	int left = GLFW_KEY_A;
	int right = GLFW_KEY_D;
	int down = GLFW_KEY_C;
	int up = GLFW_KEY_SPACE;
	int turnCamera = GLFW_MOUSE_BUTTON_1;

	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (button == turnCamera && editor::viewport->isHovered()) {
				movement::turnCamera = true;
			}
		}
		else if (action == GLFW_RELEASE) {
			if (button == turnCamera) {
				movement::turnCamera = false;
			}
		}
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (key == forward) {
				movement::forward = true;
			}
			else if (key == backward) {
				movement::backward = true;
			}
			else if (key == left) {
				movement::left = true;
			}
			else if (key == right) {
				movement::right = true;
			}
			else if (key == down) {
				movement::down = true;
			}
			else if (key == up) {
				movement::up = true;
			}
			else if (key == GLFW_KEY_P) {
				editor::actors[editor::cam - 1].cmpTransform_setPos(-10, 0.3, 2);
				editor::actors[editor::cam - 1].cmpRigidDynamic_updatePose();
				editor::actors[editor::cam - 1].cmpRigidDynamic_addForce({10, 3, 0});
			}
		}
		else if (action == GLFW_RELEASE) {
			if (key == forward) {
				movement::forward = false;
			}
			else if (key == backward) {
				movement::backward = false;
			}
			else if (key == left) {
				movement::left = false;
			}
			else if (key == right) {
				movement::right = false;
			}
			else if (key == down) {
				movement::down = false;
			}
			else if (key == up) {
				movement::up = false;
			}
		}
	}
}

void resize(GLFWwindow* window, int width, int height) {
	//editor::pbr->setViewport(width, height, 0, 0);
}

int main() {
	editor::engineBase = Zap::Base::createBase("Zap Application");
	editor::engineBase->init();

	editor::scene = new Zap::Scene();
	editor::scene->init();

	editor::window = new Zap::Window(1000, 600, "Zap Window");
	editor::window->init();
	editor::window->show();
	editor::window->setCursorPosCallback(movement::cursorPositionCallback);
	editor::window->setMousebButtonCallback(keybinds::mouseButtonCallback);
	editor::window->setKeyCallback(keybinds::keyCallback);
	editor::window->setResizeCallback(resize);

	editor::renderer = new Zap::Renderer(*editor::window);

	editor::gui = new Zap::Gui(*editor::renderer);

	Zap::ModelLoader modelLoader = Zap::ModelLoader();

	auto woodTexture = modelLoader.loadTexture("woodTexture.png");
	auto randomTexture = modelLoader.loadTexture("randomTexture.jpg");
	auto reddotsTexture = modelLoader.loadTexture("reddotsTexture.jpg");
	
	auto cubeModel = modelLoader.load("Models/OBJ/Cube.obj");
	//
#ifndef _DEBUG
	auto sponzaModel = modelLoader.load("Models/OBJ/Sponza/Sponza.obj");
#endif
	
	auto giftModel = modelLoader.load("Models/OBJ/Gift.obj");
	
	auto kimberModel = modelLoader.load("Models/OBJ/PistolKimber/PistolKimber.glb");

	auto sphereModel = modelLoader.load("Models/gltf/metalSphere.glb");

	Zap::PhysicsMaterial pxMaterial = Zap::PhysicsMaterial(0.5, 1, 0.1);

	editor::actors.push_back(Zap::Actor());
	auto pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 3.5, 0);
	pActor->addLight({3, 2, 1});
	
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 4.3, 0);
	pActor->addLight({5, 5, 5});
	
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(50, 150, 100);
	pActor->addLight({ 1700, 1000, 500 });
	
#ifndef _DEBUG
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 0, 0);
	pActor->cmpTransform_setScale(0.02, 0.02, 0.02);
	pActor->addModel(sponzaModel);
#endif
	
	// coordinate helper
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 5, 0);
	pActor->cmpTransform_setScale(0.25, 0.25, 0.25);
	pActor->addModel(cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.albedoColor = { 1, 1, 1 };
		mat.roughness = 0.5;
		mat.metallic = 0;
		mat.emissive = { 1, 1, 1, 10 };
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0.5, 5, 0);
	pActor->cmpTransform_setScale(0.5, 0.1, 0.1);
	pActor->addModel(cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.albedoColor = { 1, 0, 0 };
		mat.roughness = 0.5;
		mat.metallic = 0;
		mat.emissive = { 1, 0, 0, 5 };
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 5.5, 0);
	pActor->cmpTransform_setScale(0.1, 0.5, 0.1);
	pActor->addModel(cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.albedoColor = { 0, 1, 0 };
		mat.roughness = 0.5;
		mat.metallic = 0;
		mat.emissive = { 0, 1, 0, 5 };
		pActor->cmpModel_setMaterial(mat);
	}
	
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 5, 0.5);
	pActor->cmpTransform_setScale(0.1, 0.1, 0.5);
	pActor->addModel(cubeModel);
	{
		Zap::Material mat = Zap::Material();
		mat.albedoColor = { 0, 0, 1 };
		mat.roughness = 0.5;
		mat.metallic = 0;
		mat.emissive = { 0, 0, 1, 5 };
		pActor->cmpModel_setMaterial(mat);
	}
	//

	editor::cam = editor::actors.size();
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors[editor::cam];
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-2, 4, 0);
	pActor->cmpTransform_rotateY(-90);
	pActor->addCamera();

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 35, 0);
	pActor->cmpTransform_setScale(5, 0.5, 5);
	pActor->addModel(cubeModel);
	{
		Zap::Material mat{};
		mat.albedoColor = {1, 1, 1};
		mat.metallic = 0;
		mat.roughness = 0;
		mat.emissive = {1, 1, 1, 25};
		pActor->cmpModel_setMaterial(mat);
	}

	for (int x = 0; x < 11; x++) {
		for (int z = 0; z < 2; z++) {
			editor::actors.push_back(Zap::Actor());
			pActor = &editor::actors.back();
			editor::scene->attachActor(*pActor);
			pActor->addTransform(glm::mat4(1));
			pActor->cmpTransform_setPos((x-5)*2, 2, (z-0.5)*3);
			pActor->addModel(sphereModel);
			Zap::Material material{};
			material.albedoColor = {1, 1, 1};
			material.roughness = x/10.0;
			material.metallic = z;
			pActor->cmpModel_setMaterial(material);
		}
	}

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

	editor::mainMenuBar = new editor::MainMenuBar();
	editor::viewport = new editor::Viewport(editor::renderer, editor::scene);
	editor::sceneHierarchyView = new editor::SceneHierarchyView(editor::actors);
	editor::componentView = new editor::ComponentView();

	editor::renderer->init();

	//mainloop
	float dTime = 0;
	uint64_t frameIndex = 0;
	while (!editor::window->shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);
		
		//editor::actors[3].cmpTransform_rotateY(15*dTime);

		if (!editor::window->isIconified()) {
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

			ImGui::ShowDemoWindow();

			editor::mainMenuBar->draw();

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
			if (editor::viewport->isHovered()) windowFlags |= ImGuiWindowFlags_NoMove;
			ImGui::Begin("Viewport", nullptr, windowFlags);
			editor::viewport->draw(editor::actors[editor::cam]);
			ImGui::End();

			ImGui::Begin("Scene Hierarchy");
			editor::sceneHierarchyView->draw();
			ImGui::End();

			ImGui::Begin("ComponentView");
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2, 0.2, 0.2, 1));
			ImGui::BeginChild("Selection", ImVec2(100, 0), ImGuiChildFlags_ResizeX);
			if(editor::sceneHierarchyView->getSelectedActor())
				editor::componentView->draw(*editor::sceneHierarchyView->getSelectedActor());
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::BeginChild("Editor", ImVec2(0, 0), ImGuiChildFlags_Border);
			if (editor::componentView->getSelectedEditor()) {
				editor::componentView->getSelectedEditor()->draw(*editor::sceneHierarchyView->getSelectedActor());
			}
			ImGui::EndChild();
			ImGui::End();
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

	delete editor::viewport;
	delete editor::componentView;
	delete editor::sceneHierarchyView;

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