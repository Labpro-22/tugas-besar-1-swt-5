#ifndef SCENE_HPP
#define SCENE_HPP

class IGameFacade;
class SceneManager;

class Scene {
public:
    Scene(SceneManager* manager, IGameFacade* facade);
    virtual ~Scene() = default;

    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void onEnter() {}

protected:
    SceneManager* sceneManager;
    IGameFacade* gameFacade;
};

#endif
