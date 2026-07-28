#pragma once


class Quad
{

	static unsigned vao;
	static unsigned vbo;
	static unsigned ebo;


	static void setup();
	constexpr static float vertices[16]
	{
		-1.0f, -1.0f, 0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f, 1.0f, // bottom right
		1.0f,  1.0f, 1.0f, 1.0f, // top right
		1.0f, -1.0f, 1.0f, 0.0f // top left
	};

	constexpr static unsigned indices[6]{
		0, 1, 2,
		0, 2, 3
	};

public:
	static void draw();

};

