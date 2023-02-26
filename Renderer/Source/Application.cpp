#include "Application.h"
#include "LineDrawer.h"

using namespace Renderer;

Application::Application(const char* pTitle, const unsigned int& pWidth, const unsigned int& pHeight) : 
    mWindow(pTitle, pWidth, pHeight),
    mRenderer(mWindow),
    mCamera(pWidth, pHeight, lm::mat4::perspectiveProjection(-45, float(pWidth) / float(pHeight), 0.01f, 500.f), lm::vec3(-1, 2, 16)),
    mScene(mRenderer)
{
    mWindow.setWindowUserPointer(this);
    mRenderer.init();
    mScene.init();
}

void Application::keyCallback(int pKey, int pScancode, int pAction, int pMods)
{
    if (mWindow.getKey(GLFW_KEY_ESCAPE) == GLFW_PRESS)
        mWindow.close();
}

void Application::processInput(const float& pDeltaTime)
{
    mCamera.mIsRun = mWindow.getKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (mWindow.getKey(GLFW_KEY_A) == GLFW_PRESS)
        mCamera.processKeyboard(Renderer::CameraMovement::LEFT, pDeltaTime);
    else if (mWindow.getKey(GLFW_KEY_D) == GLFW_PRESS)
        mCamera.processKeyboard(Renderer::CameraMovement::RIGHT, pDeltaTime);

    if (mWindow.getKey(GLFW_KEY_W) == GLFW_PRESS)
        mCamera.processKeyboard(Renderer::CameraMovement::FORWARD, pDeltaTime);
    else if (mWindow.getKey(GLFW_KEY_S) == GLFW_PRESS)
        mCamera.processKeyboard(Renderer::CameraMovement::BACKWARD, pDeltaTime);
}

void Application::initScene()
{
    Model** model = mResources.create<Model>("Vempire", mRenderer, "Assets/dancing_vampire.dae");
    mLightShader = mResources.create<Shader>("shad", mRenderer, "Shader/vertex.vert.spv", "Shader/frag.frag.spv");
    Texture** texture3 = mResources.create<Texture>("text3", mRenderer, "Assets/Vampire_diffuse.png");
    GameObject* obj = new GameObject(mRenderer, mCamera, model, mLightShader, texture3, lm::vec3(0, 0, 0), lm::vec3(0, 180, 0), lm::vec3(1, 1, 1));
    mScene.addNode(obj);


    Model** model2 = mResources.create<Model>("room", mRenderer, "Assets/room.obj");
    Texture** texture2 = mResources.create<Texture>("text2", mRenderer, "Assets/room.png");
    GameObject* obj2 = new GameObject(mRenderer, mCamera, model2, mLightShader, texture2, lm::vec3(0, 0, 5), lm::vec3(90, 0, 0), lm::vec3::unitVal);
    GameObject* obj4 = new GameObject(mRenderer, mCamera, model2, mLightShader, texture2, lm::vec3(0, 0, -5), lm::vec3(90, 0, 0), lm::vec3::unitVal);
   


    Model** model3 = mResources.create<Model>("Turret", mRenderer, "Assets/Turret.obj");
    Texture** texture = mResources.create<Texture>("text1", mRenderer, "Assets/Turret.bmp");
    GameObject* obj3 = new GameObject(mRenderer, mCamera, model3, mLightShader, texture, lm::vec3(-5, 0, 0), lm::vec3(0, 0, 0), lm::vec3::unitVal);
    GameObject* obj5 = new GameObject(mRenderer, mCamera, model3, mLightShader, texture, lm::vec3(5, 0, 0), lm::vec3(0, 0, 0), lm::vec3::unitVal);
    

    obj->addChild(*obj2);
    obj->addChild(*obj4);

    obj->addChild(*obj3);
    obj->addChild(*obj5);


    mScene.addLight((new DirectionalLight(lm::vec3(0, -1, -1))));
}

void Application::updateDemo(float pDeltaTime)
{
    if (*mScene.mGameObjects[0]->mModel != nullptr)
        mScene.mGameObjects[0]->mLocal = mScene.mGameObjects[0]->mLocal * lm::mat4::yRotation(45 * pDeltaTime);

    int pair = 0;
    for (std::list<GameObject*>::iterator it = mScene.mGameObjects[0]->mChilds.begin(); it != mScene.mGameObjects[0]->mChilds.end(); it++)
    {
        if (pair & 1)
            (*it)->mLocal = (*it)->mLocal * lm::mat4::yRotation(-150 * pDeltaTime);
        else
            (*it)->mLocal = (*it)->mLocal * lm::mat4::xRotation(-150 * pDeltaTime);

        pair = (pair + 1) & 1;
    }
}

void Application::run()
{
    initScene();
    LineDrawer lineDrawer(mCamera, mRenderer, "Shader/debugVertex.vert.spv", "Shader/debugFrag.frag.spv");
    

    DeltaTime time;
    time.updateDeltaTime();
    while (!mWindow.shouldClose())
    {
        mRenderer.finishSetup();

        mWindow.pollEvents();
        time.updateDeltaTime();

        processInput(time.mDeltaTime);
        mCamera.updatePos();
        mScene.update(time.mDeltaTime);


        lineDrawer.reset();
        lineDrawer.drawLine(lm::vec3(0, 0, 0), lm::vec3(-10, 0, 0), lm::vec3(1, 0, 0));
        lineDrawer.drawLine(lm::vec3(0, 0, 0), lm::vec3(0, 0, 10), lm::vec3(0, 0, 1));
        lineDrawer.drawLine(lm::vec3(0, 0, 0), lm::vec3(0, 10, 0), lm::vec3(0, 1, 0));

        updateDemo(time.mDeltaTime);


        mRenderer.beginDraw();
        if ((mLightShader != nullptr && (*mLightShader) != nullptr))
            mScene.sendLight(*(*mLightShader));
        
        mScene.draw();

        lineDrawer.flushLines();
        mRenderer.endDraw();
    }

    mRenderer.waitForCleanUp();
    mWindow.shutDown();
    mResources.mPool.stop();
    mRenderer.finishSetup();
    mResources.clear();
    mScene.clear();
}

void Application::DeltaTime::updateDeltaTime()
{
    float currentFrame = glfwGetTime();
    mDeltaTime = currentFrame - mLastFrame;
    mLastFrame = currentFrame;
}