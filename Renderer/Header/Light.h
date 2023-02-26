#pragma once
#include "Vec4/Vec4.h"

namespace Renderer
{
	struct Light
	{
		lm::vec4 mDiffuse;
		lm::vec4 mAmbient;
		lm::vec4 mSpecular;

		Light(const lm::vec4& mDiffuse = lm::vec4(1, 1, 1, 0.8f), const lm::vec4& mAmbient = lm::vec4(1, 1, 1, 0.05f), const lm::vec4& mSpecular = lm::vec4(1, 1, 1, 1));
		bool operator==(Light& light);
	};


	struct DirectionalLight : public Light
	{
		__declspec(align(16)) lm::vec3 mDirection;

		DirectionalLight(const lm::vec3& pDirection = lm::vec3(0, -1, 0), const lm::vec4& pDiffuse = lm::vec4(1, 1, 1, 0.8f), const lm::vec4& pAmbient = lm::vec4(1, 1, 1, 0.05f), const lm::vec4& pSpecular = lm::vec4(1, 1, 1, 1));
		bool operator==(DirectionalLight& pLight);
	};

	struct PointLight : public Light
	{
		__declspec(align(16)) lm::vec3 mPosition;
		float mConstant;
		float mLinear;
		float mQuadratic;

		PointLight(const lm::vec3& pPosition = lm::vec3(0, 0, 0), float pConstant = 1.0f, float pLinear = 0.09f, float pQuadratic = 0.032f, const lm::vec4& pDiffuse = lm::vec4(1, 1, 1, 0.8f), const lm::vec4& pAmbient = lm::vec4(1, 1, 1, 0.05f), const lm::vec4& pSpecular = lm::vec4(1, 1, 1, 1));
		bool operator==(PointLight& pLight);
	};

	struct SpotLight : public Light
	{
		__declspec(align(16)) lm::vec3 mDirection;
		__declspec(align(16)) lm::vec3 mPosition;
		float mConstant;
		float mLinear;
		float mQuadratic;

		
		float mCutOff;
		float mOuterCutOff;

		SpotLight(const lm::vec3& pPosition = lm::vec3(0, 0, 0), const lm::vec3& pDirection = lm::vec3(0, 0, 1), float pCutOff = 12.5f, float pOuterCutOff = 17.5f, float pConstant = 1.0f, float pLinear = 0.09f, float pQuadratic = 0.032f, const lm::vec4& pDiffuse = lm::vec4(1, 1, 1, 0.8f), const lm::vec4& pAmbient = lm::vec4(1, 1, 1, 0.05f), const lm::vec4& pSpecular = lm::vec4(1, 1, 1, 1));
		bool operator==(SpotLight& pLight);
	};
}