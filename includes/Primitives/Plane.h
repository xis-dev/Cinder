#pragma once

#include "Graphics/Vertex.h"
namespace Plane
{

	inline std::vector<Vertex> vertices
	{
		Vertex{-0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		 Vertex{0.5f, 0.0f, 0.5f,	0.0f, 0.0f, 1.0f,1.0f, 1.0f},
		Vertex{-0.5f,0.0f, -0.5f,	0.0f, 0.0f, 1.0f,0.0f, 0.0f},
		Vertex{ 0.5f,0.0f, -0.5f,	0.0f, 0.0f, 1.0f,1.0f, 0.0f}
	};

	

	inline std::vector<unsigned int> indices
	{
		0, 1, 3,
		0, 2, 3
	};
}
