#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"


#include <string>

namespace editor {
    struct EditorData {
        std::unordered_map<Zap::UUID, std::string> actorNameMap;
        std::vector<Zap::Shape> physicsShapes;
        std::vector<Zap::PhysicsMaterial> physicsMaterials;
    };
}
