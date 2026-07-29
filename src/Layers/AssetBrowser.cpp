#include "AssetBrowser.h"

#include "Zap/Zap.h"
#include "Zap/AssetHandling/AssetHandler.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"
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

	template<class T>
	std::string typeStr();
	template<>
	std::string typeStr<Zap::Mesh>() { return "Mesh"; }
	template<>
	std::string typeStr<Zap::Material>() { return "Material"; }
	template<>
	std::string typeStr<Zap::Texture>() { return "Texture"; }

	template<class T>
	void drawTooltip(Zap::AssetHandle<T> handle) {
		if (ImGui::BeginItemTooltip()) {
			if (handle->isGenerated())
				ImGui::TextColored({ 1, 0, 0, 1 }, "Generated");
			ImGui::Text("Type: %s", typeStr<T>().c_str());
			ImGui::Text("UUID: %llu", (Zap::UUID)handle);
			ImGui::EndTooltip();
		}
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

			ImGui::Checkbox("Generated Assets", &m_settings.filterGenerated);
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
		for (auto meshIter = pAssetHandler->begin<Zap::Mesh>(); meshIter != pAssetHandler->end<Zap::Mesh>(); meshIter++) {
			auto mesh = *meshIter;
			if (!m_settings.filterGenerated && mesh->isGenerated()) continue; // skip generated assets if filtered
			ImGui::PushID(prevIndex);

			bool isButtonPressed = false;
			if (m_meshPreviewImages.count(mesh) && m_meshPreviewRefs.at(mesh))
				isButtonPressed = ImGui::ImageButton(std::to_string(mesh).c_str(), *m_meshPreviewRefs.at(mesh), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Mesh##" + std::to_string(mesh)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));
			
			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("MeshToActorPayload", &mesh, sizeof(Zap::AssetHandle<Zap::Mesh>));
				ImGui::Text("UUID: %llu", static_cast<Zap::UUID>(mesh));
				ImGui::EndDragDropSource();
			}
			else {
				drawTooltip(mesh);
			}

			nextPrev(prevIndex, newlineInterval);
			ImGui::PopID();
		}
		// Materials
		if (m_settings.filterMaterials)
		for (auto matIter = pAssetHandler->begin<Zap::Material>(); matIter != pAssetHandler->end<Zap::Material>(); matIter++) {
			auto material = *matIter;
			if (!m_settings.filterGenerated && material->isGenerated()) continue; // skip generated assets if filtered
			ImGui::PushID(prevIndex);

			bool isButtonPressed = false;
			if (m_materialPreviewImages.count(material) && m_materialPreviewRefs.at(material))
				isButtonPressed = ImGui::ImageButton(std::to_string(material).c_str(), *m_materialPreviewRefs.at(material), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Material##" + std::to_string(material)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));

			drawTooltip(material);

			nextPrev(prevIndex, newlineInterval);
			ImGui::PopID();
		}
		// Textures
		if (m_settings.filterTextures)
		for (auto texIter = pAssetHandler->begin<Zap::Texture>(); texIter != pAssetHandler->end<Zap::Texture>(); texIter++) {
			auto texture = *texIter;
			if (!m_settings.filterGenerated && texture->isGenerated()) continue; // skip generated assets if filtered
			ImGui::PushID(prevIndex);

			bool isButtonPressed = false;
			if (m_texturePreviewImages.count(texture) && m_texturePreviewRefs.at(texture))
				isButtonPressed = ImGui::ImageButton(std::to_string(texture).c_str(), *m_texturePreviewRefs.at(texture), ImVec2(m_globalSettings.previewSize.x, m_globalSettings.previewSize.y));
			else
				isButtonPressed = ImGui::Button(("Texture##" + std::to_string(texture)).c_str(), ImVec2(m_globalSettings.previewSize.x + buttonPadding.x*2, m_globalSettings.previewSize.y + buttonPadding.y*2));
			
			drawTooltip(texture);

			nextPrev(prevIndex, newlineInterval);
			ImGui::PopID();
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
		for (auto meshIter = pAssetHandler->begin<Zap::Mesh>(); meshIter != pAssetHandler->end<Zap::Mesh>(); meshIter++) {
			auto mesh = *meshIter;
			if (!m_meshPreviewImages.count(mesh)) {
				float maxLen = glm::length(mesh->getBoundMax());
				float minLen = glm::length(mesh->getBoundMin());
				float dist = std::max(maxLen, minLen); // calculate the smallest spheres radius which fully contains the mesh

				Zap::Scene scene;
				scene.init();

				auto meshMat = pAssetHandler->generateAsset<Zap::Material>(glm::vec4(1, 1, 1, 1));

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({ {mesh}, {meshMat}, {glm::mat4(1)}});

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

				m_meshPreviewImages[mesh] = renderer.extractRenderTargetImage(previewTarget);
				m_meshPreviewRefs[mesh] = std::make_unique<Zap::GuiImageRef>(m_meshPreviewImages[mesh]); // generate a reference to the preview which can be used by ImGui

				renderer.destroy();
				pAssetHandler->deleteAsset(meshMat);
				scene.destroy();
			}
		}
		//Materials
		for (auto matIter = pAssetHandler->begin<Zap::Material>(); matIter != pAssetHandler->end<Zap::Material>(); matIter++) {
			auto material = *matIter;

			if (!m_materialPreviewImages.count(material)) {
				float dist = 1.5;

				Zap::Scene scene;
				scene.init();

				Zap::Actor actor;
				scene.attachActor(actor);
				actor.addTransform(glm::mat4(1));
				actor.addModel({ {m_editorData.pDefaultMeshes->sphere}, {material}, {glm::mat4(1)} });

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

				m_materialPreviewImages[material] = renderer.extractRenderTargetImage(previewTarget);
				m_materialPreviewRefs[material] = std::make_unique<Zap::GuiImageRef>(m_materialPreviewImages[material]); // generate a reference to the preview which can be used by ImGui

				renderer.destroy();
				scene.destroy();
			}
		}
		// Textures
		for (auto texIter = pAssetHandler->begin<Zap::Texture>(); texIter != pAssetHandler->end<Zap::Texture>(); texIter++) {
			auto texture = *texIter;

			m_texturePreviewImages[texture] = std::make_shared<Zap::Image2D>(texture->getImage());
			m_texturePreviewRefs[texture] = std::make_unique<Zap::GuiImageRef>(m_texturePreviewImages[texture]);
		}
	}
}
