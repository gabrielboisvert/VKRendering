#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in ivec4 inBoneIDs;
layout(location = 7) in vec4 inWeights;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 norm;
layout(location = 2) out vec2 UV;
layout(location = 3) out vec3 view;
layout(location = 4) out vec4 fragColor;
layout(location = 5) out mat3 TBN;


const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
layout(set = 1, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
	mat3 inverseModel;
    mat4 mvp;
    vec3 view;
    mat4 finalBonesMatrices[MAX_BONES];
    bool hasAnimation;
} ubo;


void main() {
    vec4 totalPosition = vec4(0.0f);
    
    if (!ubo.hasAnimation)
        totalPosition = vec4(inPosition, 1);

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(inBoneIDs[i] == -1) 
            continue;

        if(inBoneIDs[i] >= MAX_BONES) 
        {
            totalPosition = vec4(inPosition,1.0f);
            break;
        }

        vec4 localPosition = ubo.finalBonesMatrices[inBoneIDs[i]] * vec4(inPosition,1.0f);
        totalPosition += localPosition * inWeights[i];
        
        vec3 localNormal = mat3(ubo.finalBonesMatrices[inBoneIDs[i]]) * norm;
    }

    gl_Position = ubo.mvp * totalPosition;

    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    norm = mat3(ubo.model) * inNormal;
    UV = inUV;
    view = ubo.view;
    fragColor = inColor;

    vec3 T = normalize(vec3(ubo.model * vec4(inTangent, 0.0)));
    vec3 B = normalize(vec3(ubo.model * vec4(inBitangent, 0.0)));
    vec3 N = normalize(vec3(ubo.model * vec4(inNormal, 0.0)));
    TBN = mat3(T, B, N);
}