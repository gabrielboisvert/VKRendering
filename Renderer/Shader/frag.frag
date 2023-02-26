#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec3 view;
layout(location = 4) in vec4 infragColor;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 fragColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;



const int MAX_DIR_LIGTH = 10;
//Directional light
struct DirectionalLight{
    vec4 diffuse;
	vec4 ambient;
	vec4 specular;

	vec3 direction;
};
layout(set = 0, binding = 0) buffer directionalLight
{
	int size;
	DirectionalLight data[MAX_DIR_LIGTH];
} dirLights;
//-----------------------------
//Point light
struct PointLight {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;

	vec3 position;

	float constant;
	float linear;
	float quadratic;
};
layout(set = 0, binding = 1) buffer pointLight
{
	int size;
	PointLight data[MAX_DIR_LIGTH];
} pointLights;
//-----------------------------
//Directional light
struct SpotLight {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;

	vec3 direction;
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};
layout(set = 0, binding = 2) buffer spotLight
{
	int size;
	SpotLight data[MAX_DIR_LIGTH];
} spotLights;
//-----------------------------

vec3 CalcDirLight(DirectionalLight light)
{
	float diff = max(dot(norm, -light.direction), 0.0);
	vec3 viewDir = normalize(view - fragPos);
    vec3 halfwayDir = normalize(-light.direction + viewDir);
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 518);//material.shininess);

    vec3 ambient  = light.ambient.w * vec3(light.ambient) * vec3(texture(texSampler, UV));
	vec3 diffuse  = light.diffuse.w  * diff * vec3(light.diffuse) * vec3(texture(texSampler, UV));
	vec3 specular = light.specular.w * spec * vec3(light.specular) * vec3(texture(texSampler, UV));

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);
    
    // specular shading
    vec3 viewDir = normalize(view - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 1024);//material.shininess);

    // combine results
    vec3 ambient  = light.ambient.w * vec3(light.ambient) * vec3(texture(texSampler, UV));
    vec3 diffuse  = light.diffuse.w  * diff * vec3(light.diffuse) * vec3(texture(texSampler, UV));
    vec3 specular = light.specular.w * spec * vec3(light.specular) * vec3(texture(texSampler, UV));

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse 
    float diff = max(dot(norm, lightDir), 0.0);
        
    // specular shading
    vec3 viewDir = normalize(view - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 1024);//material.shininess);  

    // combine results
    vec3 ambient  = light.ambient.w * vec3(light.ambient) * vec3(texture(texSampler, UV));
    vec3 diffuse  = light.diffuse.w  * diff * vec3(light.diffuse) * vec3(texture(texSampler, UV));
    vec3 specular = light.specular.w * spec * vec3(light.specular) * vec3(texture(texSampler, UV));

    float theta = dot(lightDir, -light.direction);
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main() 
{
    vec4 dirLightsRes = vec4(0, 0, 0, 1);
	for (int i = 0; i < dirLights.size; i++)
		dirLightsRes += vec4(CalcDirLight(dirLights.data[i]), 1);

	vec4 pointLightsRes = vec4(0, 0, 0, 1);
	for (int i = 0; i < pointLights.size; i++)
		pointLightsRes += vec4(CalcPointLight(pointLights.data[i]), 1);

    vec4 spottLightsRes = vec4(0, 0, 0, 1);
	for (int i = 0; i < spotLights.size; i++)
		spottLightsRes += vec4(CalcSpotLight(spotLights.data[i]), 1);

	fragColor = (dirLightsRes + pointLightsRes + spottLightsRes) * infragColor;
}

