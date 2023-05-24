#pragma once

#include "platform.h"
#include <glad/glad.h>

struct VertexBufferAttribute
{
	u8 location = 0;
	u8 componentCount = 0;
	u8 offset = 0;

	VertexBufferAttribute(u8 _location, u8 _componentCount, u8 _offset)
		: location(_location), componentCount(_componentCount), offset(_offset)
	{}
};

struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride = 0;

	VertexBufferLayout(u8 _stride = 0) : stride(_stride)
	{
		attributes.clear();
	}
};

struct VertexShaderAttribute
{
	u8 location = 0;
	u8 componentCount = 0;
};

struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};

struct VAO
{
	GLuint handle = 0;
	GLuint programHandle = 0;
};
