#pragma once
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <fstream>

class Mesh
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	UINT indexCount;

public:
	Mesh(Vertex* vertices,
		UINT vertexCount,
		unsigned int* indices,
		UINT _indexCount,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	Mesh(const std::wstring& objFile,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	UINT GetIndexCount();
	void Draw();
	void CreateBuffers(Vertex* verts, UINT vertexCount, unsigned int* indices, UINT indexCount, Microsoft::WRL::ComPtr<ID3D11Device> device);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
};

