#pragma once

#include "ZapEditor.h"
#include "ViewLayer.h"

namespace editor {
	class Settings : public ViewLayer
	{
	public:
		Settings(EditorData* pEditorData);
		~Settings();

		std::string name();

		void drawMain();
		
		void drawProject();

		void draw();

		ImGuiWindowFlags getWindowFlags();

		void openSectionMain();
		void openSectionProject();

	private:
		EditorData* m_pEditorData = nullptr;

		enum Sections {
			eMAIN,
			ePROJECT
		} m_section = eMAIN;
	};
}

