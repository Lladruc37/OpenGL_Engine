///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

struct Light
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) om vec3 aBitangent;

layout(binding = 0,std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};


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
	vNormal = vec3(model * vec4(aNormal,0.0));
	vViewDir = uCameraPosition - vPosition;
	gl_Position = MVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition; //in world space
in vec3 vNormal; //in world space
in vec3 vViewDir; //in world space

uniform sampler2D uTexture;

layout(binding = 0,std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location=0) out vec4 oColor;

void main()
{
	vec3 lightColor = vec3(0.0);
	for(int i = 0; i < 16;++i)
	{
	//TODO: FINISH LIGHTS
		Light light = uLight[i];
		if(light.type == 0) //directional light
		{

		}
		else //point light
		{
			lightColor += light.color;
		}
	}
	oColor = texture(uTexture,vTexCoord);
	oColor += vec4(lightColor,1.0);
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
