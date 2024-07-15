#include "AssetBrowser.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"

namespace editor {
	std::unordered_map<Zap::UUID, Zap::GuiTexture> AssetBrowser::m_meshPreviews = {};
	std::unordered_map<Zap::UUID, Zap::Image> AssetBrowser::m_meshPreviewImages = {};

	AssetBrowser::AssetBrowser(Zap::Window* pWindow, Zap::Gui* pGui)
		: m_pWindow(pWindow), m_pGui(pGui)
	{
		reloadPreviews();
	}

	AssetBrowser::~AssetBrowser() {

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
				scene.init();

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({"", {Zap::Material()}, {meshPair.first}});

				Zap::Actor cam;
				scene.attachActor(cam);
				cam.addTransform(glm::mat4(1));
				cam.cmpTransform_setPos({0, 0, -5});
				cam.addCamera();

				Zap::Renderer renderer = Zap::Renderer(*m_pWindow);
				auto pbRender = new Zap::PBRenderer(renderer, &scene);
				pbRender->setViewport(1, 1, 0, 0);
				pbRender->setRenderTarget(&image);
				pbRender->updateCamera(cam);
				renderer.addRenderTemplate(pbRender);
				renderer.init();

				renderer.beginRecord();
				renderer.recRenderTemplate(pbRender);
				renderer.endRecord();

				renderer.render();

				image.changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
				m_meshPreviews[meshPair.first] = m_pGui->loadTexture(&image);
			}
		}
	}
}
