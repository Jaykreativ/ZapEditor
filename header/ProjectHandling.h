#pragma once

#include "ZapEditor.h"

namespace editor {
	namespace project {
		void create(EditorData& editorData, std::string name, std::string directory);

		void open(EditorData& editorData, std::string name, std::string directory);
		void open(EditorData& editorData, std::string filepath);

		void close(EditorData& editorData);

		void save(EditorData& editorData);
	}
}

