#pragma once

namespace Zap{
	class PBRenderer;
	class Renderer;
}

namespace editor {
	class Viewport
	{
	public:
		bool canMove = true;

		Viewport(Zap::PBRenderer* render, Zap::Renderer* renderer);

		void updateGui();

	private:
		Zap::PBRenderer* m_render;
		Zap::Renderer* m_renderer;
	};
}

