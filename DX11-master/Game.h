#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Mesh.h"
#include "Camera.h"
#include "gameEntity.h"
#include <memory>
#include <vector>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "SimpleShader.h"
#include "Lights.h"
#include "Sky.h"

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> setCubeSRV(const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
	void CreateShadowMap();
	void RenderShadowMap();
	void setupPP();
private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();

	//Shapes
	std::vector<std::shared_ptr<Mesh>> meshes;
	//cameras
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCam;
	//materials
	std::shared_ptr<Material> bronzeMat;
	std::shared_ptr<Material> cobblestoneMat;
	std::shared_ptr<Material> paintMat;
	std::shared_ptr<Material> scratchedMat;
	std::shared_ptr<Material> woodMat;


	std::shared_ptr<Sky> skyBox;


	//entities
	std::vector<std::shared_ptr<gameEntity>> gameEntities;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> customPShader;

	std::shared_ptr<SimpleVertexShader> skyVertexShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;

	DirectX::XMFLOAT3 ambientColor;
	Light directionalLight1;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze_albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze_metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze_normals;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze_roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone_albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone_metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone_normals;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone_roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paint_albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paint_metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paint_normals;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paint_roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratched_albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratched_metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratched_normals;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratched_roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood_albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood_metal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood_normals;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood_roughness;


	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	std::shared_ptr<SimpleVertexShader> shadowVShader;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;

	// Resources that are tied to a particular post process
	std::shared_ptr<SimplePixelShader> blurPPPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;
	int blur;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> colorRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorSRV;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ambientRTV; 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientSRV; 

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> normalsRTV; 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalsSRV; 

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> depthRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthSRV; 

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SSAORTV; 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SSAOSRV;
	std::shared_ptr<SimplePixelShader> ppssaoPS;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurredSSAORTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>blurredSSAOSRV;
	std::shared_ptr<SimplePixelShader> ppssaoblurPS;
	std::shared_ptr<SimplePixelShader> combinePS;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>randomTextureSRV;

	DirectX::XMFLOAT4 ssaoOffsets[64];

	float ssaoRadius;
	int ssaoSamples;
};


