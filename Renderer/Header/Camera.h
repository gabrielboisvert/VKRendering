#pragma once
#include "Mat4/Mat4.h"

namespace Renderer
{
	enum class CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class Camera
	{
		public:
			struct MouseHandler
			{
				bool mFirstMouse = true;

				float mLastX = 0;
				float mLastY = 0;

				float mMouseX = 0;
				float mMouseY = 0;
			};

			lm::vec3 mPosition;
			float mYaw = -90;
			float mPitch = 0;

			lm::vec3 mForward = lm::vec3::forward;
			lm::vec3 mRight = lm::vec3::right;
			lm::vec3 mUp = lm::vec3::up;
			
			lm::mat4 mView;
			lm::mat4 mProjection;
			lm::mat4 mVp;
			
			float mSpeed = 20;
			float mSpeedMultiplier = 3;
			float mSensitivity = 0.1f;
			bool mIsRun = false;

			MouseHandler mMouse;

			Camera(const float pWidth, const float pHeight, const lm::mat4& pProjection, const lm::vec3& pPosition);

			lm::vec3 calculateFront();
			void changeViewPortSize(const lm::mat4& pProjection);
			void updatePos();
			void updateCameraVectors();
			void processKeyboard(CameraMovement pDirection, float pDeltaTime);
			void setMousePosition(float pMouseX, float pMouseY);
	};
}