#pragma once

#include "Zap/Rendering/RenderObjects/RenderTasks/Gui.h"

#include "ZapEditor.h"
#include "ViewLayer.h"

namespace editor {
	struct AssetBrowserSettings {
		bool filterMeshes = true;
		bool filterMaterials = true;
		bool filterTextures = true;
	};

	struct StaticAssetBrowserSettings {
		glm::vec2 previewSize = { 150, 150 };
	};

	class AssetBrowser : public ViewLayer
	{
	public:
		AssetBrowser(EditorData& editorData);
		~AssetBrowser();

		std::string name() override;

		void draw() override;

		ImGuiWindowFlags getWindowFlags() override;
		
		void clearPreviews();

		void loadPreviews();

	private:
		AssetBrowserSettings m_settings = {};
		static StaticAssetBrowserSettings m_globalSettings;

		EditorData& m_editorData;

		std::unordered_map<Zap::UUID, std::unique_ptr<Zap::GuiImageRef>> m_meshPreviewRefs = {};
		std::unordered_map<Zap::UUID, std::shared_ptr<Zap::Image2D>> m_meshPreviewImages = {};

		std::unordered_map<Zap::UUID, std::unique_ptr<Zap::GuiImageRef>> m_materialPreviewRefs = {};
		std::unordered_map<Zap::UUID, std::shared_ptr<Zap::Image2D>> m_materialPreviewImages = {};

		std::unordered_map<Zap::UUID, std::unique_ptr<Zap::GuiImageRef>> m_texturePreviewRefs = {};
		std::unordered_map<Zap::UUID, std::shared_ptr<Zap::Image2D>> m_texturePreviewImages = {};
	};
}

