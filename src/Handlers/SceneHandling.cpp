#include "SceneHandling.h"

#include "Zap/Serializer.h"

#include <sstream>
#include <fstream>
#include <iostream>

#define ZP_SCENE_FILE_EXTENSION ".zscn"

namespace editor {
	SceneReference::operator Zap::Scene&() {
		return getData()->scene;
	}
	SceneReference::operator Zap::Scene*() {
		return &getData()->scene;
	}

	bool SceneReference::operator==(SceneReference& other) {
		return this->operator Zap::Scene*() == other.operator Zap::Scene*();
	}
	Zap::Scene* SceneReference::operator->() {
		return *this;
	}

	bool SceneReference::expired() {
		return !getData().operator bool();
	}

	Zap::Actor SceneReference::createActor(std::string name) {
		Zap::Actor actor; // create new actor
		getData()->scene.attachActor(actor);
		actor.addName(name);
		getData()->actors.push_back(actor); // add actor to the editor
		return actor;
	}

	void SceneReference::destroyActor(Zap::Actor actor) {
		size_t i = 0;
		for (auto other : getData()->actors) {
			if (other == actor)
				return destroyActor(i);
			i++;
		}
	}
	void SceneReference::destroyActor(size_t index) {
		//delete actor
		getData()->actors[index].destroy();
		getData()->actors.erase(getData()->actors.begin() + index);
	}

	std::vector<Zap::Actor>& SceneReference::actors() {
		return getData()->actors;
	}

	SceneIterator::SceneIterator(size_t index) : m_index(index) {}
	void SceneIterator::operator++(int) {
		m_index++;
	}
	bool SceneIterator::operator==(const SceneIterator& it) {
		return m_index == it.m_index;
	}
	bool SceneIterator::operator!=(const SceneIterator& it) {
		return !(*this == it);
	}
	SceneIterator::operator size_t() {
		return m_index;
	}

	SceneHandler::~SceneHandler() {
		for (auto& data : m_sceneData)
			data->scene.destroy();
	}

	std::shared_ptr<SceneHandler::SceneData> SceneHandler::get(SceneIterator it) {
		return m_sceneData[it];
	}

	CustomSceneReference SceneHandler::create(std::string name) {
		m_sceneData.push_back(std::make_shared<SceneData>(name));
		auto& scene = m_sceneData.back()->scene;
		scene.init();
		SceneIterator it(m_sceneData.size() - 1);
		if (m_sceneData.size() == 1)
			activate(it); // activate the first scene by default
		return getReference(it);
	}

	CustomSceneReference SceneHandler::load(std::filesystem::path path, LoadFailFlags* pFail) {
		std::ifstream file(path);

		if (!file.good()) { // check filepath
			ZP_WARN(false, ("invalid filepath: " + path.string() + " | Scene:Handler::loadScene").c_str());
			if (pFail)
				*pFail |= eInvalidFilepath;
			return CustomSceneReference(std::weak_ptr<SceneHandler::SceneData>()); // return expired reference
		}

		Zap::UUID sceneID; // check for duplicates
		Zap::Serializer::readSceneIDReadable(sceneID, file);
		auto duplicateRef = getDuplicateByID(sceneID);
		if (!duplicateRef.expired()) {
			if(pFail)
				*pFail |= eDuplicate;
			return duplicateRef;
		}
		file.seekg(0); // go back to beginning

		auto scene = create(path.filename().replace_extension().string());
		Zap::Serializer::readSceneReadable(scene, path, file);
		file.close();
		scene.actors() = scene->scanActors();
		return scene;
	}

	SceneIterator SceneHandler::begin() {
		return SceneIterator(0);
	}
	SceneIterator SceneHandler::end() {
		return SceneIterator(m_sceneData.size());
	}

	void SceneHandler::save(std::filesystem::path path, SceneIterator it) {
		path = path.replace_filename(get(it)->scene.name() + ZP_SCENE_FILE_EXTENSION);
		auto& scene = get(it)->scene;
		std::ofstream file(path);
		if (!file.good()) {
			ZP_WARN(false, ("invalid filepath: " + path.string() + " | Scene:Handler::saveScene").c_str());
			return;
		}
		Zap::Serializer::writeSceneReadable(scene, path, file);
		file.close();
	}

	void SceneHandler::destroy(SceneIterator it) {
		get(it)->scene.destroy();
		m_sceneData.erase(m_sceneData.begin() + it);
	}

	std::string SceneHandler::getName(SceneIterator it) {
		return get(it)->scene.name();
	}

	CustomSceneReference SceneHandler::getReference(SceneIterator it) {
		return CustomSceneReference(get(it));
	}

	void SceneHandler::activate(SceneIterator it) {
		if (m_active.lock() == get(it)) // ignore unchanged
			return;
		ActiveSceneChangeEvent event(CustomSceneReference(m_active), getActiveReference());
		m_active = get(it);		m_eventHandler.pushEvent(event);
	}

	bool SceneHandler::isActive(SceneIterator it) {
		return m_active.lock() == get(it);
	}

	std::string SceneHandler::getActiveName() {
		if (auto sp = m_active.lock())
			return sp->scene.name();
		return "None";
	}

	ActiveSceneReference SceneHandler::getActiveReference() {
		return ActiveSceneReference(*this);
	}

	SceneHandlerEventHandler& SceneHandler::getEventHandler() {
		return m_eventHandler;
	}

	CustomSceneReference SceneHandler::getDuplicateByID(Zap::UUID id) {
		for (auto it = begin(); it != end(); it++) {
			if (id == get(it)->scene.getHandle())
				return getReference(it);
		}
		return CustomSceneReference(std::weak_ptr<SceneHandler::SceneData>());
	}

	ActiveSceneReference::ActiveSceneReference(SceneHandler& handler)
		: m_handler(handler)
	{}

	std::shared_ptr<SceneHandler::SceneData> ActiveSceneReference::getData() {
		return m_handler.m_active.lock();
	}

	CustomSceneReference::CustomSceneReference(std::weak_ptr<SceneHandler::SceneData> wptr)
		: m_wptr(wptr)
	{}

	std::shared_ptr<SceneHandler::SceneData> CustomSceneReference::getData() {
		return m_wptr.lock();
	}
}
