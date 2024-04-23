#include "Viewport.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/EventHandler.h"
#include "Zap/Rendering/PathTacer.h"
#include "Zap/Rendering/Gui.h"
#include "Zap/Scene/Scene.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include <chrono>

void cursorPositionCallbackDef(GLFWwindow* window, double xpos, double ypos, void* data) {
	auto obj = (editor::Viewport*)data;
	obj->cursorPositionCallback(window, xpos, ypos);
}

namespace editor {
	Viewport::Viewport(Zap::Renderer* pRenderer, Zap::Scene* pScene, Zap::EventHandler* pEventHandler)
		: m_pRenderer(pRenderer), m_pEventHandler(pEventHandler)
	{
		m_outImage.setFormat(Zap::GlobalSettings::getColorFormat());
		m_outImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_outImage.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_outImage.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_outImage.setWidth(1);
		m_outImage.setHeight(1);
		
		m_outImage.init();
		m_outImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_outImage.initView();

		m_pbRender = new Zap::PBRenderer(*m_pRenderer, pScene);
		m_pbRender->setViewport(1, 1, 0, 0);
		m_pbRender->setRenderTarget(&m_outImage);
		m_pRenderer->addRenderTemplate(m_pbRender);

		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_rtxRender = new Zap::RaytracingRenderer(*m_pRenderer, pScene);
			m_rtxRender->setRenderTarget(&m_outImage);
			m_pRenderer->addRenderTemplate(m_rtxRender);

			m_pathTracer = new Zap::PathTracer(*m_pRenderer, pScene);
			m_pathTracer->setRenderTarget(&m_outImage);
			m_pRenderer->addRenderTemplate(m_pathTracer);
		}

		m_sampler.init();

		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);
		
		m_pEventHandler->addCursorPositionCallback(cursorPositionCallbackDef, this);

		m_camera = Zap::Actor();
		pScene->attachActor(m_camera);
		m_camera.addTransform(glm::mat4(1));
		m_camera.addCamera();
	}

	Viewport::~Viewport() {
		delete m_pbRender;
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			delete m_rtxRender;
			delete m_pathTracer;
		}
		m_sampler.destroy();
		m_outImage.destroy();
	}

	std::string Viewport::name() {
		return "Viewport";
	}

	int forward = GLFW_KEY_W;// move keybinds to editor::preferences
	int backward = GLFW_KEY_S;
	int left = GLFW_KEY_A;
	int right = GLFW_KEY_D;
	int down = GLFW_KEY_C;
	int up = GLFW_KEY_SPACE;
	int turnCamera = GLFW_MOUSE_BUTTON_1;
	void Viewport::move(float dTime) {
		if (m_pEventHandler->isKeyPressed(forward)) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (m_pEventHandler->isKeyPressed(backward)) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (m_pEventHandler->isKeyPressed(right)) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (m_pEventHandler->isKeyPressed(left)) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (m_pEventHandler->isKeyPressed(down)) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (m_pEventHandler->isKeyPressed(up)) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
	}

	double xlast = 0;
	double ylast = 0;
	float sensitivityX = 0.2;
	float sensitivityY = 0.15;
	void Viewport::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		if (m_pEventHandler->isMouseButtonPressed(turnCamera) && isHovered()) {
			glm::mat4 res = m_camera.cmpTransform_getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>((xpos - xlast) * sensitivityX), glm::vec3{ 0, 1, 0 });

			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];

			m_camera.cmpTransform_setTransform(res);
			m_camera.cmpTransform_rotateX((ypos - ylast) * sensitivityY);
		}

		xlast = xpos;
		ylast = ypos;
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
		auto imageExtent = m_outImage.getExtent();
		auto extent = ImGui::GetContentRegionAvail();
		if (extent.x != imageExtent.width || extent.y != imageExtent.height) {// resize
			extent.x = std::max<float>(extent.x, 1);
			extent.y = std::max<float>(extent.y, 1);
			m_outImage.setWidth(extent.x);
			m_outImage.setHeight(extent.y);
			m_pbRender->setViewport(extent.x, extent.y, 0, 0);
			update();
		}
		ImGui::Image(m_imageDescriptorSet, extent);
		m_isHovered = ImGui::IsItemHovered();

		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		move(dTime);
		dTime = std::chrono::duration_cast<std::chrono::duration<float>>(timeEndFrame - timeStartFrame).count();
		timeStartFrame = std::chrono::high_resolution_clock::now();

		m_pbRender->updateCamera(m_camera);
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_rtxRender->updateCamera(m_camera);
			m_pathTracer->updateCamera(m_camera);
		}
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
		m_pRenderer->beginRecord();
		m_pbRender->disable();
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_rtxRender->disable();
			m_pathTracer->disable();
		}
		switch (m_renderType)
		{
		case ePBR:
			m_pbRender->resize();
			m_pRenderer->recRenderTemplate(m_pbRender);
			m_pRenderer->recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
			m_pbRender->enable();
			break;
		case eRAYTRACING:
			m_rtxRender->resize();
			m_pRenderer->recRenderTemplate(m_rtxRender);
			m_rtxRender->enable();
			break;
		case ePATHTRACING:
			m_pathTracer->resize();
			m_pRenderer->recRenderTemplate(m_pathTracer);
			m_pathTracer->enable();
			break;
		default:
			break;
		}
		m_pRenderer->endRecord();
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

	}
}