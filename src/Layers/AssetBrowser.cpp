#include "AssetBrowser.h"

#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/RenderObjects/RenderTasks/PBRenderer.h"

namespace editor {
	StaticAssetBrowserSettings AssetBrowser::m_globalSettings = {};

	AssetBrowser::AssetBrowser(EditorData& editorData)
		: m_editorData(editorData)
	{
		loadPreviews();
	}

	AssetBrowser::~AssetBrowser() {}

	std::string AssetBrowser::name() {
		return "AssetBrowser";
	}

	void nextPrev(size_t &i, int newlineInterval) {
		i++;
		if (newlineInterval!=0 && i % newlineInterval!=0)
			ImGui::SameLine();
	}

	void AssetBrowser::draw() {
		auto style = ImGui::GetStyle();
		auto buttonPadding = style.FramePadding;
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("View")) {
				if (ImGui::BeginMenu("Settings")) {
					int prevSize = m_globalSettings.previewSize.x;
					ImGui::DragInt("Preview Size", &prevSize, 1, 1, 1080);
					m_globalSettings.previewSize = { prevSize, prevSize };
					
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::Button("Reload");
			if (ImGui::BeginItemTooltip()) {
				ImGui::Text("Render previews for unloaded assets. Right-click to reload all previews.");
				ImGui::EndTooltip();
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				loadPreviews();
			}
			else if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				clearPreviews();
				loadPreviews();
			}

			ImGui::Checkbox("Meshes", &m_settings.filterMeshes);
			ImGui::Checkbox("Materials", &m_settings.filterMaterials);
			ImGui::Checkbox("Textures", &m_settings.filterTextures);

			ImGui::EndMenuBar();
		}
		
		size_t prevIndex = 0;
		float xAvailSize = ImGui::GetContentRegionAvail().x;
		int newlineInterval = (xAvailSize - style.ItemSpacing.x) / (style.FramePadding.x * 2 + style.ItemSpacing.x + m_globalSettings.previewSize.x); // amount of previews per line

		// Meshes
		if(m_settings.filterMeshes)
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			const auto meshId = meshPair.first;
			bool isButtonPressed = false;
			if (m_meshPreviewImages.count(meshId) && m_meshPreviewRefs.at(meshId))
				isButtonPressed = ImGui::ImageButton(std::to_string(meshId).c_str(), *m_meshPreviewRefs.at(meshId), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Mesh##" + std::to_string(meshId)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("MeshToActorPayload", &meshId, sizeof(Zap::UUID));
				ImGui::EndDragDropSource();
			}
			nextPrev(prevIndex, newlineInterval);
		}
		// Materials
		if (m_settings.filterMaterials)
		for (auto matIter = pAssetHandler->beginMaterials(); matIter != pAssetHandler->endMaterials(); matIter++) {
			const auto& matPair = *matIter;
			const auto matId = matPair.first;
			bool isButtonPressed = false;
			if (m_materialPreviewImages.count(matId) && m_materialPreviewRefs.at(matId))
				isButtonPressed = ImGui::ImageButton(std::to_string(matId).c_str(), *m_materialPreviewRefs.at(matId), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Material##" + std::to_string(matId)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));
			nextPrev(prevIndex, newlineInterval);
		}
		// Textures
		if (m_settings.filterTextures)
		for (auto texIter = pAssetHandler->beginTextures(); texIter != pAssetHandler->endTextures(); texIter++) {
			const auto& texPair = *texIter;
			const auto texId = texPair.first;
			bool isButtonPressed = false;
			if (m_texturePreviewImages.count(texId) && m_texturePreviewRefs.at(texId))
				isButtonPressed = ImGui::ImageButton(std::to_string(texId).c_str(), *m_texturePreviewRefs.at(texId), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Texture##" + std::to_string(texId)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));
			nextPrev(prevIndex, newlineInterval);
		}
	}

	ImGuiWindowFlags AssetBrowser::getWindowFlags() {
		return 0;
	}

	void AssetBrowser::clearPreviews() {
		m_meshPreviewRefs.clear();
		m_meshPreviewImages.clear();
		m_materialPreviewRefs.clear();
		m_materialPreviewImages.clear();
		m_texturePreviewRefs.clear();
		m_texturePreviewImages.clear();
	}

	void AssetBrowser::loadPreviews() {
		auto* base = Zap::Base::getBase();
		auto* pAssetHandler = base->getAssetHandler();
		// Meshes
		for (auto meshIter = pAssetHandler->beginMeshes(); meshIter != pAssetHandler->endMeshes(); meshIter++) {
			const auto& meshPair = *meshIter;
			const auto meshId = meshPair.first;
			if (!m_meshPreviewImages.count(meshId)) {
				float maxLen = glm::length(meshPair.second.m_boundMax);
				float minLen = glm::length(meshPair.second.m_boundMin);
				float dist = std::max(maxLen, minLen); // calculate the smallest spheres radius which fully contains the mesh

				Zap::Scene scene;
				scene.init();

				Zap::Material meshMat;

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({"", {meshMat}, {meshId}});

				Zap::Actor light1;
				scene.attachActor(light1);
				light1.addTransform(glm::mat4(1));
				light1.cmpTransform_setPos(glm::vec3(.5, .6, -1)*dist);
				light1.addLight({1, 1, 1}, 5*dist*dist);
				light1.cmpLight_setRadius(0);

				Zap::Actor light2;
				scene.attachActor(light2);
				light2.addTransform(glm::mat4(1));
				light2.cmpTransform_setPos(glm::vec3(1, 1.5, .5)*dist);
				light2.addLight({1, .8, .3}, 3*dist*dist);
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
				meshMat.remove();
				scene.destroy();
			}
		}
		//Materials
		for (auto matIter = pAssetHandler->beginMaterials(); matIter != pAssetHandler->endMaterials(); matIter++) {
			auto& matPair = *matIter;
			auto matId = matPair.first;

			if (!m_materialPreviewImages.count(matId)) {
				float dist = 1.5;

				Zap::Scene scene;
				scene.init();

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({ "", {matId}, {m_editorData.pDefaultMeshes->sphere} });

				Zap::Actor light1;
				scene.attachActor(light1);
				light1.addTransform(glm::mat4(1));
				light1.cmpTransform_setPos(glm::vec3(.5, .6, -1) * dist);
				light1.addLight({ 1, 1, 1 }, 5 * dist * dist);
				light1.cmpLight_setRadius(0);

				Zap::Actor light2;
				scene.attachActor(light2);
				light2.addTransform(glm::mat4(1));
				light2.cmpTransform_setPos(glm::vec3(1, 1.5, .5) * dist);
				light2.addLight({ 1, .8, .3 }, 3 * dist * dist);
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

				m_materialPreviewImages[matId] = renderer.extractRenderTargetImage(previewTarget);
				m_materialPreviewRefs[matId] = std::make_unique<Zap::GuiImageRef>(m_materialPreviewImages[matId]); // generate a reference to the preview which can be used by ImGui

				renderer.destroy();
				scene.destroy();
			}
		}
		// Textures
		for (auto texIter = pAssetHandler->beginTextures(); texIter != pAssetHandler->endTextures(); texIter++) {
			const auto& texPair = *texIter;
			const auto texId = texPair.first;
			const auto texture = texPair.second;

			m_texturePreviewImages[texId] = std::make_shared<Zap::Image2D>(texture.image);
			m_texturePreviewRefs[texId] = std::make_unique<Zap::GuiImageRef>(m_texturePreviewImages[texId]);
		}
	}
}
