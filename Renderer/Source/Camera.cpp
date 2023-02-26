#include "Camera.h"


Renderer::Camera::Camera(const float pWidth, const float pHeight, const lm::mat4& pProjection, const lm::vec3& pPosition) 
    : mProjection(pProjection),
      mPosition(pPosition)
{
    mMouse.mLastX = pWidth * 0.5f;
    mMouse.mLastY = pHeight * 0.5f;
}

lm::vec3 Renderer::Camera::calculateFront()
{
    lm::vec3 front;
    front.X() = cos(mYaw * (M_PI / HALF_CIRCLE));
    front.Y() = sin(mPitch * (M_PI / HALF_CIRCLE));
    front.Z() = sin(mYaw * (M_PI / HALF_CIRCLE));
    front.normalize();
    return front;
}

void Renderer::Camera::setMousePosition(float pMouseX, float pMouseY)
{
    if (mMouse.mFirstMouse)
    {
        mMouse.mLastX = pMouseX;
        mMouse.mLastY = pMouseY;
        mMouse.mFirstMouse = false;
    }

    float xoffset = pMouseX - mMouse.mLastX;
    float yoffset = mMouse.mLastY - pMouseY;
    mMouse.mLastX = pMouseX;
    mMouse.mLastY = pMouseY;

    xoffset *= mSensitivity;
    yoffset *= mSensitivity;

    mYaw -= xoffset;
    mPitch += yoffset;

    if (mPitch > 90.0f)
        mPitch = 90.0f;
    if (mPitch < -90.0f)
        mPitch = -90.0f;
    
    updateCameraVectors();
}

void Renderer::Camera::changeViewPortSize(const lm::mat4& pProjection)
{
    mProjection = pProjection;
}

void Renderer::Camera::updateCameraVectors()
{
    lm::vec3 front;
    front.X() = cos(mYaw * (M_PI / HALF_CIRCLE)) * cos(mPitch * (M_PI / HALF_CIRCLE));
    front.Y() = sin(mPitch * (M_PI / HALF_CIRCLE));
    front.Z() = sin(mYaw * (M_PI / HALF_CIRCLE)) * cos(mPitch * (M_PI / HALF_CIRCLE));
    mForward = front.normalized();

    mRight = mForward.crossProduct(lm::vec3::up).normalized();
    mUp = mRight.crossProduct(mForward).normalized();
}

void Renderer::Camera::processKeyboard(CameraMovement pDirection, float pDeltaTime)
{
    float velocity = mSpeed * (mIsRun? mSpeedMultiplier : 1) * pDeltaTime;
    
    lm::vec3 front = calculateFront();

    if (pDirection == CameraMovement::FORWARD)
        mPosition += front * velocity;
    if (pDirection == CameraMovement::BACKWARD)
        mPosition -= front * velocity;
    if (pDirection == CameraMovement::LEFT)
        mPosition += mRight * velocity;
    if (pDirection == CameraMovement::RIGHT)
        mPosition -= mRight * velocity;
}

void Renderer::Camera::updatePos()
{
	mView = lm::mat4::lookAt(mPosition, mPosition + mForward, mUp);
	mVp = mProjection * mView;
}