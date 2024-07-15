#pragma once

#include "imgui.h"
#include <string>

namespace editor {
	class ViewLayer {
	public:
		ViewLayer() = default;

		virtual ~ViewLayer() = default;

		virtual std::string name() = 0;

		virtual void draw() = 0;
		
		virtual ImGuiWindowFlags getWindowFlags() = 0;
	};
}
