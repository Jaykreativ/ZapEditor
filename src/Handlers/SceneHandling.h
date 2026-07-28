#pragma once

#include "ZapEditor.h"

#include "Zap/Scene/Actor.h"

namespace editor {
	class SceneIterator {
	public:
		~SceneIterator() = default;
		SceneIterator(const SceneIterator& other) = default;

		void operator++(int);
		bool operator==(const SceneIterator& it);
		bool operator!=(const SceneIterator& it);
		operator size_t();

	private:
		SceneIterator(size_t index);
		size_t m_index = 0;

		friend class SceneHandler;
	};

	class SceneHandler {
		friend class SceneReference;
		friend class ActiveSceneReference;
		friend class CustomSceneReference;
	public:
		SceneHandler() = default;
		~SceneHandler();

		CustomSceneReference create(std::string name);

		CustomSceneReference load(std::filesystem::path path);

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
			SceneData(std::string name) : name(name) {}
			std::string name;
			Zap::Scene scene;
			std::vector<Zap::Actor> actors;

			// custom ECS components
			std::unordered_map<Zap::UUID, std::string> actorNameMap = {};
		};
		std::vector<std::shared_ptr<SceneData>> m_sceneData;
		std::weak_ptr<SceneData> m_active;

		std::shared_ptr<SceneData> get(SceneIterator it);
	};

	class SceneReference {
		virtual std::shared_ptr<SceneHandler::SceneData> getData() = 0;
	public:
		operator Zap::Scene& ();
		operator Zap::Scene* ();

		bool operator==(SceneReference& other);
		Zap::Scene* operator->();

		bool expired();

		Zap::Actor createActor(std::string name);

		void destroyActor(Zap::Actor actor);
		void destroyActor(size_t index);

		void renameActor(Zap::Actor actor, std::string name);

		std::string actorName(Zap::Actor actor);

		std::vector<Zap::Actor>& actors();
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