#include "Viewport.h"

#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/EventHandler.h"
#include "Zap/Rendering/PathTacer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include <chrono>

namespace editor {
	int forward    = GLFW_KEY_W;// move keybinds to editor::preferences
	int backward   = GLFW_KEY_S;
	int right      = GLFW_KEY_D;
	int left       = GLFW_KEY_A;
	int down       = GLFW_KEY_C;
	int up         = GLFW_KEY_SPACE;
	int turnCamera = GLFW_MOUSE_BUTTON_1;

	bool forwardPressed    = false;
	bool backwardPressed   = false;
	bool rightPressed      = false;
	bool leftPressed       = false;
	bool downPressed       = false;
	bool upPressed         = false;
	bool turnCameraPressed = false;

	void mouseButtonCallback(Zap::MouseButtonEvent& params, void* data) {
		if (params.action == GLFW_PRESS) {
			if (params.button == turnCamera)
				turnCameraPressed = true;
		}
		else if (params.action == GLFW_RELEASE) {
			if (params.button == turnCamera)
				turnCameraPressed = false;
		}
	}

	void keyCallback(Zap::KeyEvent& params, void* data) {
		if (params.action == GLFW_PRESS) {
			if (params.key == forward) {
				forwardPressed = true;
			}
			else if (params.key == backward) {
				backwardPressed = true;
			}
			else if (params.key == right) {
				rightPressed = true;
			}
			else if (params.key == left) {
				leftPressed = true;
			}
			else if (params.key == down) {
				downPressed = true;
			}
			else if (params.key == up) {
				upPressed = true;
			}
		}
		else if (params.action == GLFW_RELEASE) {
			if (params.key == forward) {
				forwardPressed = false;
			}
			else if (params.key == backward) {
				backwardPressed = false;
			}
			else if (params.key == right) {
				rightPressed = false;
			}
			else if (params.key == left) {
				leftPressed = false;
			}
			else if (params.key == down) {
				downPressed = false;
			}
			else if (params.key == up) {
				upPressed = false;
			}
		}
	}

	Viewport::Viewport(Zap::Scene* pScene, Zap::Window* pWindow)
		: m_pScene(pScene), m_pWindow(pWindow)
	{
		m_pWindow->getKeyEventHandler()->addCallback(keyCallback);
		m_pWindow->getMouseButtonEventHandler()->addCallback(mouseButtonCallback);
		m_pWindow->getCursorPosEventHandler()->addCallback(Viewport::cursorPositionCallback, this);

		m_outImage.setFormat(Zap::GlobalSettings::getColorFormat());
		m_outImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_outImage.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_outImage.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_outImage.setWidth(1);
		m_outImage.setHeight(1);
		
		m_outImage.init();
		m_outImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_outImage.initView();

		m_pPBRender = new Zap::PBRenderer(pScene);
		m_pPBRender->setViewport(1, 1, 0, 0);

		m_renderer.setTarget(&m_outImage);
		m_renderer.addRenderTask(m_pPBRender);
		m_renderer.beginRecord();
		m_renderer.recRenderTemplate(m_pPBRender);
		m_renderer.endRecord();

		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender = new Zap::RaytracingRenderer(pScene);
			m_renderer.addRenderTask(m_pRTRender);
		
			m_pPathTracer = new Zap::PathTracer(pScene);
			m_renderer.addRenderTask(m_pPathTracer);
		}

		m_renderer.init();

		m_sampler.init();

		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

		m_camera = Zap::Actor();
		pScene->attachActor(m_camera);
		m_camera.addTransform(glm::mat4(1));
		m_camera.addCamera();
	}

	Viewport::~Viewport() {
		m_renderer.destroy();
		m_sampler.destroy();
		m_outImage.destroy();

		m_pWindow->getKeyEventHandler()->removeCallback(keyCallback);
		m_pWindow->getMouseButtonEventHandler()->removeCallback(mouseButtonCallback);
		m_pWindow->getCursorPosEventHandler()->removeCallback(Viewport::cursorPositionCallback, this);
	}

	std::string Viewport::name() {
		return "Viewport";
	}

	void Viewport::move(float dTime) {
		if (forwardPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (backwardPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (rightPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (leftPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (downPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (upPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
	}

	double xlast = 0;
	double ylast = 0;
	float sensitivityX = 0.2;
	float sensitivityY = 0.15;
	void Viewport::cursorPositionCallback(Zap::CursorPosEvent& params, void* viewportData) {
		Viewport* pViewport = (Viewport*)viewportData;
		if (turnCameraPressed && pViewport->isHovered()) {
			glm::mat4 res = pViewport->m_camera.cmpTransform_getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>((params.xPos - xlast) * sensitivityX), glm::vec3{ 0, 1, 0 });
			
			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];
			
			pViewport->m_camera.cmpTransform_setTransform(res);
			pViewport->m_camera.cmpTransform_rotateX((params.yPos - ylast) * sensitivityY);
		}
		
		xlast = params.xPos;
		ylast = params.yPos;
	}

	float dTime = 0;
	std::chrono::steady_clock::time_point timeStartFrame;
	void Viewport::draw() {
		if (ImGui::BeginMenuBar()) {
			std::string mode;
			switch (m_renderType)
			{
			case ePBR:
				mode = "PBR";
				break;
			case eRAYTRACING:
				mode = "Raytracing";
				break;
			case ePATHTRACING:
				mode = "Path Tracing";
				break;
			default:
				break;
			}
			if (ImGui::BeginMenu(("Mode: " + mode).c_str())) {
				if (ImGui::MenuItem("PBR")) {
					changeRenderType(ePBR);
				}

				// disable rendering modes that use raytracing if not enabled
				if (!Zap::Base::getBase()->getSettings()->enableRaytracing)
					ImGui::BeginDisabled();

				if (ImGui::MenuItem("Raytracing")) {
					changeRenderType(eRAYTRACING);
				}
				if (ImGui::MenuItem("Path Tracing")) {
					changeRenderType(ePATHTRACING);
				}

				if (!Zap::Base::getBase()->getSettings()->enableRaytracing)
					ImGui::EndDisabled();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		//Resize outImage and dependencies
		auto imageExtent = m_outImage.getExtent();
		auto extent = ImGui::GetContentRegionAvail();
		if (extent.x != imageExtent.width || extent.y != imageExtent.height) {// resize
			extent.x = std::max<float>(extent.x, 1);
			extent.y = std::max<float>(extent.y, 1);
			m_outImage.setWidth(extent.x);
			m_outImage.setHeight(extent.y);
			m_pPBRender->setViewport(extent.x, extent.y, 0, 0);
			update();
		}
		ImGui::Image(m_imageDescriptorSet, extent);
		m_isHovered = ImGui::IsItemHovered();

		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		move(dTime);
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		timeStartFrame = std::chrono::high_resolution_clock::now();

		m_pPBRender->updateCamera(m_camera);
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender->updateCamera(m_camera);
			m_pPathTracer->updateCamera(m_camera);
		}

		m_pScene->update();
		m_renderer.render();
	}

	ImGuiWindowFlags Viewport::getWindowFlags() {
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
		if (m_isHovered) windowFlags |= ImGuiWindowFlags_NoMove;
		return windowFlags;
	}

	void Viewport::changeRenderType(RenderType renderType) {
		m_renderType = renderType;
		update();
	}

	void Viewport::update() {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		m_outImage.update();
		m_renderer.beginRecord();
		m_pPBRender->disable();
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender->disable();
			m_pPathTracer->disable();
		}
		switch (m_renderType)
		{
		case ePBR:
			m_renderer.recRenderTemplate(m_pPBRender);
			m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
			m_pPBRender->enable();
			break;
		case eRAYTRACING:
			m_renderer.recRenderTemplate(m_pRTRender);
			m_pRTRender->enable();
			break;
		case ePATHTRACING:
			m_renderer.recRenderTemplate(m_pPathTracer);
			m_pPathTracer->enable();
			break;
		default:
			break;
		}
		m_renderer.endRecord();
		m_renderer.resize();
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

	}
}