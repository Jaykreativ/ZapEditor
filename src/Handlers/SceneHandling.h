#pragma once

#include "ZapEditor.h"

#include "Zap/Scene/Actor.h"

namespace editor {
	class SceneReference {
		virtual std::shared_ptr<SceneHandler::SceneData> getData() = 0;
	public:
	};

	class SceneIterator {
	public:
		~SceneIterator() = default;
		SceneIterator(const SceneIterator& other) = default;

		void operator++(int);
		bool operator==(const SceneIterator& it);
		bool operator!=(const SceneIterator& it);

	private:
		SceneIterator(size_t index);
		size_t m_index = 0;

		operator size_t();

		friend class SceneHandler;
	};

	class SceneHandler {
		friend class SceneReference;
		friend class ActiveSceneReference;
		friend class CustomSceneReference;
	public:
		Zap::Scene& create(std::string name);

		Zap::Scene* load(std::filesystem::path path);

		SceneIterator begin();
		SceneIterator end();

		void save(std::filesystem::path path, SceneIterator it);

		void destroy(SceneIterator it);

		std::string getName(SceneIterator it);

		CustomSceneReference getReference(SceneIterator it);

		void activate(SceneIterator it);

		bool isActive(SceneIterator it);

		std::string getActiveName();

		ActiveSceneReference getActiveReference();

	private:
		struct SceneData {
			std::string name;
			Zap::Scene scene;
			std::vector<Zap::Actor> actors;
		};
		std::vector<std::shared_ptr<SceneData>> m_sceneData;
		std::weak_ptr<SceneData> m_active;

		std::shared_ptr<SceneData> get(SceneIterator it);
	};

	class ActiveSceneReference : public SceneReference {
		SceneHandler& m_handler;

		ActiveSceneReference(SceneHandler& handler);

		virtual std::shared_ptr<SceneHandler::SceneData> getData() override;

		friend class SceneHandler;
	};

	class CustomSceneReference : public SceneReference {
		std::weak_ptr<SceneHandler::SceneData> m_wptr;

		CustomSceneReference(std::weak_ptr<SceneHandler::SceneData> wptr);

		virtual std::shared_ptr<SceneHandler::SceneData> getData() override;

		friend class SceneHandler;
	};
}