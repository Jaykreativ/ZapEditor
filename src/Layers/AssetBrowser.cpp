#include "AssetBrowser.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/PBRenderer.h"

namespace editor {
	StaticAssetBrowserSettings AssetBrowser::m_globalSettings = {};

	AssetBrowser::AssetBrowser()
	{
		loadPreviews();
	}

	AssetBrowser::~AssetBrowser() {}

	std::string AssetBrowser::name() {
		return "AssetBrowser";
	}

	void AssetBrowser::draw() {
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();
		
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			const auto meshId = meshPair.first;
			if (!m_meshPreviewImages.count(meshId))
				continue;
			if (ImGui::ImageButton(std::to_string(meshId).c_str(), *m_meshPreviewRefs.at(meshId), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y))) {
			
			}
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("MeshToActorPayload", &meshId, sizeof(Zap::UUID));
				ImGui::EndDragDropSource();
			}
		}
	}

	ImGuiWindowFlags AssetBrowser::getWindowFlags() {
		return 0;
	}

	void AssetBrowser::loadPreviews() {
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			const auto meshId = meshPair.first;
			if (!m_meshPreviewImages.count(meshId)) {
				float maxLen = glm::length(meshPair.second.m_boundMax);
				float minLen = glm::length(meshPair.second.m_boundMin);
				float dist = std::max(maxLen, minLen); // calculate the smallest spheres radius which fully contains the mesh

				Zap::Scene scene;
				scene.init();

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({"", {Zap::Material()}, {meshId}});

				Zap::Actor light1;
				scene.attachActor(light1);
				light1.addTransform(glm::mat4(1));
				light1.cmpTransform_setPos(glm::vec3(.5, .6, -1)*dist);
				light1.addLight({1, 1, 1}, 10*dist*dist);
				light1.cmpLight_setRadius(0);

				Zap::Actor light2;
				scene.attachActor(light2);
				light2.addTransform(glm::mat4(1));
				light2.cmpTransform_setPos(glm::vec3(1, 1.5, .5)*dist);
				light2.addLight({1, .8, .3}, 6*dist*dist);
				light2.cmpLight_setRadius(0);

				Zap::Actor cam;
				scene.attachActor(cam);
				cam.addTransform(glm::mat4(1));
				glm::mat4 camOffset = glm::mat4(1);
				auto camPos = glm::vec3(0.6, 1.1, -1.5) * dist;
				camOffset[3] = glm::vec4(camPos, 1);
				cam.addCamera(camOffset);
				cam.cmpCamera_lookAtCenter();

				scene.update();

				Zap::Renderer renderer;
				// create the preview image
				auto previewTarget = renderer.createRenderTarget<Zap::RenderTargetImage>(
					Zap::GlobalSettings::getColorFormat(),
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				//create pbr task
				auto pbrTask = renderer.createRenderTask<Zap::PBRenderer>(previewTarget, &scene);
				pbrTask->clearColor = { .1, .1, .1, 1 };

				renderer.beginRecord();
				renderer.recRenderTask(pbrTask);
				renderer.endRecord();
				renderer.resize(m_globalSettings.previewSize);
				pbrTask->setViewport(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y, 0, 0);
				pbrTask->updateCamera(cam);
				
				renderer.render();

				m_meshPreviewImages[meshId] = renderer.extractRenderTargetImage(previewTarget);
				m_meshPreviewRefs[meshId] = std::make_unique<Zap::GuiImageRef>(m_meshPreviewImages[meshId]); // generate a reference to the preview which can be used by ImGui

				renderer.destroy();
				scene.destroy();

			}
		}
	}
}
