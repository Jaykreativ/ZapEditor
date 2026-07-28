#include "SceneHandling.h"

#define ZP_SCENE_FILE_EXTENSION ".zscn"

namespace editor {
	void SceneIterator::operator++(int) {
		m_index++;
	}
	bool SceneIterator::operator==(const SceneIterator& it) {
		m_index == it.m_index;
	}
	bool SceneIterator::operator!=(const SceneIterator& it) {
		return !(*this == it);
	}
	SceneIterator::operator size_t() {
		return m_index;
	}

	std::shared_ptr<SceneHandler::SceneData> SceneHandler::get(SceneIterator it) {
		return m_sceneData[it];
	}

	Zap::Scene& SceneHandler::create(std::string name) {
		m_sceneData.push_back(std::make_shared<SceneData>(name));
		auto& scene = m_sceneData.back()->scene;
		scene.init();
		return scene;
	}

	Zap::Scene* SceneHandler::load(std::filesystem::path path) {
		auto& scene = create(path.filename().replace_extension().string());
		std::ifstream file(path);
		if (!file.good()) {
			ZP_WARN(false, ("invalid filepath: " + path.string() + " | Scene:Handler::loadScene").c_str());
			return nullptr;
		}
		Zap::Serializer::readSceneReadable(scene, path, file);
		file.close();
		return &scene;
	}

	SceneIterator SceneHandler::begin() {
		return SceneIterator(0);
	}
	SceneIterator SceneHandler::end() {
		return SceneIterator(m_sceneData.size());
	}

	void SceneHandler::save(std::filesystem::path path, SceneIterator it) {
		path = path.replace_filename(get(it)->name + ZP_SCENE_FILE_EXTENSION);
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
		return get(it)->name;
	}

	CustomSceneReference SceneHandler::getReference(SceneIterator it) {
		return CustomSceneReference(get(it));
	}

	void SceneHandler::activate(SceneIterator it) {
		m_active = get(it);
	}

	bool SceneHandler::isActive(SceneIterator it) {
		return m_active.lock() == get(it);
	}

	std::string SceneHandler::getActiveName() {
		if (auto sp = m_active.lock())
			return sp->name;
		return "None";
	}

	ActiveSceneReference SceneHandler::getActiveReference() {
		return ActiveSceneReference(*this);
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
