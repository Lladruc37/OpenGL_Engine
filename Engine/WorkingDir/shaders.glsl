///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

struct Light
{
	unsigned int type;
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
};

layout(binding = 0,std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) om vec3 aBitangent;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 model;
	mat4 MVP;
};

out vec2 vTexCoord;
out vec3 vPosition; //in world space
out vec3 vNormal; //in world space
out vec3 vViewDir; //in world space

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(model * vec4(aPosition,1.0));
	vNormal = mat3(transpose(inverse(model))) * aNormal;
	vViewDir = normalize(uCameraPosition - vPosition);
	gl_Position = MVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition; //in world space
in vec3 vNormal; //in world space
in vec3 vViewDir; //in world space

struct Material
{
	sampler2D diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

layout(location=0) out vec4 oColor;

void main()
{
	vec3 norm = normalize(vNormal);
	vec3 result = vec3(0.0);

	for(int i = 0; i < uLightCount;++i)
	{
		Light light = uLight[i];
		vec3 lightDir = vec3(0.0);
		vec3 reflectDir = reflect(-lightDir, norm);
		float attenuation = 1.0;

		if(light.type == 0) //directional light
		{
			lightDir = normalize(-light.direction);
		}
		else //point light
		{
			lightDir = normalize(light.position - vPosition);
			float linear = 0.09;
			float quadratic = 0.032;
			//-----------
			float dist = length(light.position - vPosition);
			attenuation = 1.0/(light.constant + linear * dist + quadratic * (dist * dist));
		}
		vec3 ambient = light.ambient * vec3(texture(material.diffuse, vTexCoord));

		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse,vTexCoord));

		float spec = pow(max(dot(vViewDir, reflectDir), 0.0), /*material.shininess*/32);
		vec3 specular = light.specular * spec * material.specular;

		result += (ambient + diffuse + specular) * attenuation;
	}
	oColor = vec4(result,1.0);
}

#endif
#endif

#ifdef TEXTURED_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location=0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture,vTexCoord);
}

#endif
#endif

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
