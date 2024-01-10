#include "SceneHierarchy.h";
#include "ComponentView.h";

#include "Zap/Zap.h"
#include "Zap/ModelLoader.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Model.h"
#include "imgui.h"
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

	static Zap::Scene* scene;

	static uint32_t cam = 0;
	static std::vector<Zap::Actor> actors;

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
			if (button == turnCamera) {
				if (!ImGui::GetIO().WantCaptureMouse) {
					movement::turnCamera = true;
				}
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
			/*else if (key == GLFW_KEY_ENTER) {
				glm::vec3 dir = app::cam.getTransform()[2];
				px::cubePxActor->addForce(PxVec3(dir.x*500, dir.y*500, dir.z*500));
				//px::cubePxActor->setGlobalPose(PxTransform(PxVec3(0, 5, 0)));
			}*/
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
	editor::pbr->setViewport(width, height, 0, 0);
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

	editor::pbr = new Zap::PBRenderer(*editor::renderer, editor::scene);


	Zap::ModelLoader modelLoader = Zap::ModelLoader();

	auto cubeModel = modelLoader.load("Models/OBJ/Cube.obj");

#ifndef _DEBUG
	auto sponzaModel = modelLoader.load("Models/OBJ/Sponza.obj");
#endif

	auto giftModel = modelLoader.load("Models/OBJ/Gift.obj");

	editor::actors.push_back(Zap::Actor());
	auto pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, -1, 0);
	pActor->cmpTransform_setScale(25, 1, 25);
	pActor->addModel(cubeModel);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 3.5, 0);
	pActor->addLight({3, 2, 1});

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-2, 1, -0.5);
	pActor->addLight({2, 2, 2});

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 1, -3);
	pActor->addModel(giftModel);

	// coordinate helper
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 1, 0);
	pActor->cmpTransform_setScale(0.25, 0.25, 0.25);
	pActor->addModel(cubeModel);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0.5, 1, 0);
	pActor->cmpTransform_setScale(0.5, 0.1, 0.1);
	pActor->addModel(cubeModel);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 1.5, 0);
	pActor->cmpTransform_setScale(0.1, 0.5, 0.1);
	pActor->addModel(cubeModel);

	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors.back();
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(0, 1, 0.5);
	pActor->cmpTransform_setScale(0.1, 0.1, 0.5);
	pActor->addModel(cubeModel);
	//

	editor::cam = editor::actors.size();
	editor::actors.push_back(Zap::Actor());
	pActor = &editor::actors[editor::cam];
	editor::scene->attachActor(*pActor);
	pActor->addTransform(glm::mat4(1));
	pActor->cmpTransform_setPos(-1, 1, -5);
	pActor->addCamera();

	editor::pbr->setViewport(1000, 600, 0, 0);
	editor::renderer->addRenderTemplate(editor::pbr);
	editor::renderer->addRenderTemplate(editor::gui);
	editor::renderer->init();

	editor::sceneHierarchyView = new editor::SceneHierarchyView(editor::actors);
	editor::componentView = new editor::ComponentView();

	//mainloop
	float dTime = 0;
	uint64_t frameIndex = 0;
	while (!editor::window->shouldClose()) {
		auto timeStartFrame = std::chrono::high_resolution_clock::now();
		movement::move(dTime);
		
		editor::actors[3].cmpTransform_rotateY(15*dTime);

		ImGui::ShowDemoWindow();

		ImGui::Begin("Scene Hierarchy");
		editor::sceneHierarchyView->draw();
		ImGui::End();

		ImGui::Begin("ComponentView");
		if(editor::sceneHierarchyView->getSelectedActor())
			editor::componentView->draw(*editor::sceneHierarchyView->getSelectedActor());
		ImGui::BeginChild("Editor");
		if(editor::componentView->getSelectedEditor())
			editor::componentView->getSelectedEditor()->draw(*editor::sceneHierarchyView->getSelectedActor());
		ImGui::EndChild();
		ImGui::End();

		if (dTime > 0) {
			editor::scene->simulate(dTime);
		}

		editor::pbr->updateBuffers(editor::actors[editor::cam]);
		editor::renderer->render();

		Zap::Window::pollEvents();
		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		frameIndex++;
	}

	delete editor::componentView;
	delete editor::sceneHierarchyView;

	//terminate
	editor::renderer->destroy();
	delete editor::renderer;
	delete editor::window;

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