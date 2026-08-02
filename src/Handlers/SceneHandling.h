#pragma once

#include "ZapEditor.h"

#include "Zap/Events.h"
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

	class ActiveSceneChangeEvent;
	class SceneHandlerEventHandler :
		public Zap::EventHandler<ActiveSceneChangeEvent>
	{
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

		enum LoadFailBits {
			eSuccess = 0x0,
			eDuplicate = 0x1,
			eInvalidFilepath = 0x2
		};
		typedef uint32_t LoadFailFlags;
		// creates a new scene and fills it with the scene data specified by path
		// the result can be checked using pFail flags
		// eDuplicate: returns a ref to the duplicate
		// eInvalidFilepath: returns an expired ref
		CustomSceneReference load(std::filesystem::path path, LoadFailFlags* pFail = nullptr);

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

		SceneHandlerEventHandler& getEventHandler();

		CustomSceneReference getDuplicateByID(Zap::UUID id);

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

		SceneHandlerEventHandler m_eventHandler;

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

		std::string name();

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

	class ActiveSceneChangeEvent : public Zap::Event {
	public:
		CustomSceneReference lastActive;
		ActiveSceneReference nowActive;
		ActiveSceneChangeEvent(CustomSceneReference lastActive, ActiveSceneReference nowActive) : lastActive(lastActive), nowActive(nowActive) {}
	};
}