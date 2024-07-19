#include "AssetBrowser.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"

namespace editor {
	AssetBrowser::AssetBrowser(Zap::Window* pWindow, Zap::Gui* pGui)
		: m_pWindow(pWindow), m_pGui(pGui)
	{
		reloadPreviews();
	}

	AssetBrowser::~AssetBrowser() {
		for (auto& previewPair : m_meshPreviews)
			m_pGui->unloadTexture(previewPair.second);
		for (auto& imagePair : m_meshPreviewImages)
			imagePair.second.destroy();
	}

	std::string AssetBrowser::name() {
		return "AssetBrowser";
	}

	void AssetBrowser::draw() {
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();
		
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			if (ImGui::ImageButton(std::to_string(meshPair.first).c_str(), m_meshPreviews.at(meshPair.first), ImVec2(m_previewSize.x, m_previewSize.y))) {

			}
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("MeshToActorPayload", &meshPair.first, sizeof(Zap::UUID));
				ImGui::EndDragDropSource();
			}
		}
	}

	ImGuiWindowFlags AssetBrowser::getWindowFlags() {
		return 0;
	}

	void AssetBrowser::reloadPreviews() {
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			if (!m_meshPreviews.count(meshPair.first)) {
				Zap::Image& image = (m_meshPreviewImages[meshPair.first] = Zap::Image());
				image.setFormat(Zap::GlobalSettings::getColorFormat());
				image.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
				image.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				image.setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
				image.setWidth(m_previewSize.x);
				image.setHeight(m_previewSize.y);
			
				image.init();
				image.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				image.initView();

				Zap::Scene scene = Zap::Scene();

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({"", {Zap::Material()}, {meshPair.first}});

				Zap::Actor light;
				scene.attachActor(light);
				light.addTransform(glm::mat4(1));
				light.cmpTransform_setPos({1, 1, -3});
				light.addLight({1, 1, 1}, 10);

				Zap::Actor cam;
				scene.attachActor(cam);
				cam.addTransform(glm::mat4(1));
				glm::mat4 camOffset = glm::mat4(1);
				camOffset[3] = glm::vec4(2, 3, -5, 1);
				cam.addCamera(camOffset);
				cam.cmpCamera_lookAtCenter();

				scene.init();

				Zap::Renderer renderer;
				auto* pbRender = new Zap::PBRenderer(&scene);
				pbRender->setViewport(m_previewSize.x, m_previewSize.y, 0, 0);
				pbRender->updateCamera(cam);
				pbRender->clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
				renderer.setTarget(&image);
				renderer.addRenderTask(pbRender);
				renderer.beginRecord();
				renderer.recRenderTemplate(pbRender);
				renderer.endRecord();
				renderer.init();
				
				scene.update();
				renderer.render();

				renderer.destroy();
				scene.destroy();

				image.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
				m_meshPreviews[meshPair.first] = m_pGui->loadTexture(&image);
			}
		}
	}
}
