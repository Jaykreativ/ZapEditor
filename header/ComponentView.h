#pragma once

#include "Zap/Scene/Actor.h"

namespace editor {
	class ComponentView
	{
	public:
		ComponentView();
		~ComponentView();

		void draw(Zap::Actor selectedActor);
	};
}