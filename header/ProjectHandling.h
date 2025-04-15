#pragma once

#include "ZapEditor.h"

#include <filesystem>

namespace editor {
	namespace project {
		void create(EditorData& editorData, std::string name, std::string directory);

		void open(EditorData& editorData, std::string name, std::string directory);
		void open(EditorData& editorData, std::filesystem::path filepath);

		void close(EditorData& editorData);

		void save(EditorData& editorData);
	}
}

