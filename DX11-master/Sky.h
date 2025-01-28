#pragma once
#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"
#include "WICTextureLoader.h"

class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySrv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;
	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;
public:
	Sky(std::shared_ptr<Mesh> _mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		std::shared_ptr<SimpleVertexShader> _vertexShader,
		std::shared_ptr<SimplePixelShader> _pixelShader,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

		// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.
	//
	// - Note: This code assumes you’re putting the function in Sky.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Device> device);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> cam, Microsoft::WRL::ComPtr<ID3D11Device> device);
};

