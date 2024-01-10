#pragma once

#include "Zap/Scene/Actor.h"

namespace editor {
	class ComponentEditor
	{
	public:
		virtual void draw(Zap::Actor selectedActor) = 0;
	};

	class TransformEditor : public ComponentEditor
	{
	public:
		void draw(Zap::Actor selectedActor);
	};

	class ComponentView
	{
	public:
		ComponentView();
		~ComponentView();

		void draw(Zap::Actor selectedActor);

		ComponentEditor* getSelectedEditor();
	private:
		ComponentEditor* m_selectedEditor;
	};
}