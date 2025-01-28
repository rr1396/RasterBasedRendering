#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include <unordered_map>

class Material
{
private: 
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	float roughness;
	float offset;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

public:
	Material(DirectX::XMFLOAT4 _colorTint, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness);
	
	std::shared_ptr<SimpleVertexShader> getVertexShader();
	std::shared_ptr<SimplePixelShader> getPixelShader();
	DirectX::XMFLOAT4 getColorTint();
	float getRoughness();

	void setVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader);
	void setPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void setColorTint(DirectX::XMFLOAT4 _colorTint);
	void setShaders(DirectX::XMFLOAT4X4 worldMatrix,
		DirectX::XMFLOAT4X4 viewMatrix,
		DirectX::XMFLOAT4X4 projectionMatrix,
		DirectX::XMFLOAT4X4 worldInverseTransposeMatrix,
		DirectX::XMFLOAT3 position);
	void AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
};

