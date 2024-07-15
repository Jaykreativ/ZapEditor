#pragma once

#include "Zap/Rendering/Gui.h"

#include "ViewLayer.h"

namespace editor {
	class AssetBrowser : public ViewLayer
	{
	public:
		AssetBrowser(Zap::Window* pWindow, Zap::Gui* pGui);
		~AssetBrowser();

		std::string name() override;

		void draw() override;

		ImGuiWindowFlags getWindowFlags() override;

		void reloadPreviews();

	private:
		Zap::Window* m_pWindow;
		Zap::Gui* m_pGui;

		glm::vec2 m_previewSize = { 50, 50 };

		static std::unordered_map<Zap::UUID, Zap::GuiTexture> m_meshPreviews;
		static std::unordered_map<Zap::UUID, Zap::Image> m_meshPreviewImages;
	};
}

