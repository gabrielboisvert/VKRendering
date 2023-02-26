#include "Light.h"

using namespace Renderer;


Light::Light(const lm::vec4& pDiffuse, const lm::vec4& pAmbient, const lm::vec4& pSpecular) : mDiffuse(pDiffuse), mAmbient(pAmbient), mSpecular(pSpecular)
{
}

bool Renderer::Light::operator==(Light& pLight)
{
	return mDiffuse == pLight.mDiffuse && mAmbient == pLight.mAmbient && mSpecular == pLight.mSpecular;
}

DirectionalLight::DirectionalLight(const lm::vec3& pDirection, const lm::vec4& pDiffuse, const lm::vec4& pAmbient, const lm::vec4& pSpecular)
	: Light(pDiffuse, pAmbient, pSpecular), mDirection(pDirection)
{
}

bool Renderer::DirectionalLight::operator==(DirectionalLight& pLight)
{
	return mDiffuse == pLight.mDiffuse && mAmbient == pLight.mAmbient && mSpecular == pLight.mSpecular && mDirection == pLight.mDirection;
}

PointLight::PointLight(const lm::vec3& pPosition, float pConstant, float pLinear, float pQuadratic, const lm::vec4& pDiffuse, const lm::vec4& pAmbient, const lm::vec4& pSpecular) 
	: Light(pDiffuse, pAmbient, pSpecular), mPosition(pPosition), mConstant(pConstant), mLinear(pLinear), mQuadratic(pQuadratic)
{
}

bool Renderer::PointLight::operator==(PointLight& pLight)
{
	return mDiffuse == pLight.mDiffuse && mAmbient == pLight.mAmbient && mSpecular == pLight.mSpecular && mPosition == pLight.mPosition && mConstant == pLight.mConstant && mLinear == pLight.mLinear && mQuadratic == pLight.mQuadratic;
}

SpotLight::SpotLight(const lm::vec3& pPosition, const lm::vec3& pDirection, float pCutOff, float pOuterCutOff, float pConstant, float pLinear, float pQuadratic, const lm::vec4& pDiffuse, const lm::vec4& pAmbient, const lm::vec4& pSpecular)
	: Light(pDiffuse, pAmbient, pSpecular), mDirection(pDirection), mPosition(pPosition), mConstant(pConstant), mLinear(pLinear), mQuadratic(pQuadratic), mCutOff(pCutOff), mOuterCutOff(pOuterCutOff)
{
}

bool Renderer::SpotLight::operator==(SpotLight& pLight)
{
	return mDiffuse == pLight.mDiffuse && mAmbient == pLight.mAmbient && mSpecular == pLight.mSpecular && mDirection == pLight.mDirection && mPosition == pLight.mPosition && mConstant == pLight.mConstant && mLinear == pLight.mLinear && mQuadratic == pLight.mQuadratic && mCutOff == pLight.mCutOff && mOuterCutOff == pLight.mOuterCutOff;
}
