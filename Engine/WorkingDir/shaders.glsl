///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef POST_PROCESSING_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec2 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,0.0,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
uniform sampler2D finalImage;

layout(location=0) out vec4 finalColor;

void main()
{
	finalColor = texture(finalImage,vTexCoord);
}

#endif
#endif

#ifdef GEOMETRY_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 model;
	mat4 MVP;
};

out vec2 vTexCoord;
out vec3 vPosition; //in world space
out vec3 vNormal; //in world space

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(model * vec4(aPosition,1.0));
	vNormal = mat3(transpose(inverse(model))) * aNormal;
	gl_Position = MVP * vec4(aPosition, 1.0);
}


#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gSpec;

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;

struct Material
{
	sampler2D diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

void main()
{
	// store the fragment position vector in the first gbuffer texture
    gPosition = vec4(vPosition,1.0);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(vNormal),1.0);
    // and the diffuse per-fragment color
    gAlbedo = texture(material.diffuse, vTexCoord);
    // store specular intensity in gAlbedoSpec's alpha component
    gSpec = vec4(material.specular,1.0);
}

#endif
#endif

#ifdef LIGHTING_PASS

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

layout(location=0) in vec2 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,0.0,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
uniform int renderTarget;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpec;

layout(location=0) out vec4 gColor;

void main()
{
	vec3 fragPos = texture(gPosition,vTexCoord).rgb;
	vec3 normal = texture(gNormal,vTexCoord).rgb;
	vec3 albedo = texture(gAlbedo, vTexCoord).rgb;
	float specularTex = texture(gSpec,vTexCoord).r;

	vec3 lighting = vec3(0.0);//albedo * 0.1;
	vec3 viewDir = normalize(uCameraPosition - fragPos);
	for(int i = 0; i < uLightCount; ++i)
	{
		Light light = uLight[i];
		vec3 lightDir = vec3(0.0);
		vec3 reflectDir = reflect(-lightDir, normal);
		float attenuation = 1.0;

		if(light.type == 0) //directional light
		{
			lightDir = normalize(-light.direction);
		}
		else //point light
		{
			lightDir = normalize(light.position - fragPos);
			float linear = 0.09;
			float quadratic = 0.032;
			//-----------
			float dist = length(light.position - fragPos);
			attenuation = 1.0/(light.constant + linear * dist + quadratic * (dist * dist));
		}
		vec3 ambient = light.ambient * albedo;

		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = light.diffuse * diff * albedo;

		float spec = pow(max(dot(viewDir, reflectDir), 0.0), /*material.shininess*/32);
		vec3 specular = light.specular * spec * specularTex;

		lighting += (ambient + diffuse + specular) * attenuation;
	}

	
	switch(renderTarget)
	{
		case 0: //final
		{
			gColor = vec4(lighting,1.0);
			break;
		}
		case 1: //position
		{
			gColor = vec4(fragPos,1.0);
			break;
		}
		case 2: //normal
		{
			gColor = vec4(normal,1.0);
			break;
		}
		case 3: //albedo
		{
			gColor = vec4(albedo,1.0);
			break;
		}
		case 4: //spec
		{
			gColor = vec4(vec3(specularTex),1.0);
			break;
		}
	}
}

#endif
#endif


#ifdef FORWARD

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

layout(location=0) out vec4 gColor;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gSpec;

void main()
{
	// store the fragment position vector in the first gbuffer texture
    gPosition = vec4(vPosition,1.0);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(vNormal),1.0);
    // and the diffuse per-fragment color
    gAlbedo = texture(material.diffuse, vTexCoord);
    // store specular intensity in gAlbedoSpec's alpha component
    gSpec = vec4(material.specular,1.0);


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
	gColor = vec4(result,1.0);
}

#endif
#endif

#ifdef TEXTURED_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec2 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,0.0,1.0);
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
