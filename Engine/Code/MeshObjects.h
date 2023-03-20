#pragma once

#define H 32
#define V 16

static const float pi = 3.1416f;
struct Vertex { QVector3D pos; QVector3D norm; };

Vertex sphere[H][V + 1];
for (int h = 0; h < H; ++h)
{
	for (int v = 0; < V; ++v)
	{
		float nh = float(h) / H;
		float nv = float(v) / V - 0.5f;
		float angleh = 2 * pi * nh;
		float anglev = -pi * nv;
		sphere[h][v].pos.setX(sinf(angleh)* cosf(anglev));
		sphere[h][v].pos.setY(-sinf(anglev));
		sphere[h][v].pos.setZ(cosf(angleh)* cosf(anglev));
		sphere[h][v].norm = sphere[h][v].pos;
	}
}

unsigned int sphereIndices[H][V][6];
for (unsigned int h = 0; h < H; ++h)
{
	for (unsigned int v = 0; v < V; ++v)
	{
		sphereIndices[h][v][0] = (h + 0) * (v + 1) + v;
		sphereIndices[h][v][1] = ((h + 1) % H) * (v + 1) + v;
		sphereIndices[h][v][2] = ((h + 1) % H) * (v + 1) + v + 1;
		sphereIndices[h][v][3] = (h + 0) * (v + 1) + v;
		sphereIndices[h][v][4] = ((h + 1) % H) * (v + 1) + v + 1;
		sphereIndices[h][v][5] = (h + 0) * (v + 1) + v + 1;
	}
}

VertexFormat vertexFormat;
vertexFormat.setVertexAttribute(0, 0, 3);
vertexFormat.setVertexAttribute(1, sizeof(QVector3D), 3);

Mesh* mesh = createMesh();
mesh->name = "Sphere";
mesh->addSubMesh(vertexFormat, sphere, sizeof(sphere), &sphereIndices[0][0][0], H* V * 6);
this->sphere = mesh;