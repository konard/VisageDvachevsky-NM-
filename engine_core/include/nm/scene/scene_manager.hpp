#pragma once

#include "nm/core/types.hpp"
#include "nm/core/result.hpp"
#include "nm/scene/scene_object.hpp"
#include <memory>
#include <vector>
#include <string>

namespace nm::scene
{

enum class LayerType
{
    Background,
    Characters,
    UI,
    Effects
};

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    Result<void> loadScene(const std::string& sceneId);
    void unloadScene();

    void update(f64 deltaTime);
    void render(renderer::IRenderer& renderer);

    void addToLayer(LayerType layer, std::unique_ptr<SceneObject> object);
    void removeFromLayer(LayerType layer, const std::string& objectId);
    void clearLayer(LayerType layer);

    SceneObject* findObject(const std::string& id);

    [[nodiscard]] const std::string& getCurrentSceneId() const;

private:
    std::vector<std::unique_ptr<SceneObject>>& getLayer(LayerType layer);

    std::string m_currentSceneId;
    std::vector<std::unique_ptr<SceneObject>> m_backgroundLayer;
    std::vector<std::unique_ptr<SceneObject>> m_charactersLayer;
    std::vector<std::unique_ptr<SceneObject>> m_uiLayer;
    std::vector<std::unique_ptr<SceneObject>> m_effectsLayer;
};

} // namespace nm::scene
