#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "material.h"
#include "WICTextureLoader.h"
#include "Sky.h"


// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>



// For the DirectX Math library
using namespace DirectX;

// Helper macro for getting a float between min and max
#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
	gameEntities = std::vector<std::shared_ptr<gameEntity>>();
	cameras = std::vector<std::shared_ptr<Camera>>();
	meshes = std::vector<std::shared_ptr<Mesh>>();
	ambientColor = XMFLOAT3(0.1f,0.1f,0.25f);
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	//ImGui cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	gameEntities.clear();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
	skyBox = std::make_shared<Sky>(meshes[5], sampler, device, skyVertexShader, 
		skyPixelShader, context, FixPath(L"../../Assets/Textures/Clouds_Pink/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds_Pink/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds_Pink/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds_Pink/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds_Pink/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds_Pink/back.png").c_str());

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		//context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame

			
	}

	//Initialize ImGui and platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	//pick a style
	ImGui::StyleColorsDark();
	//also comes in StyleColorsLight and StyleColorsClassic;

		//Get Size as the next multiple of 16 (instead of hardcoding a size here)
	cameras.push_back(std::make_shared<Camera>((float)this->windowWidth / this->windowHeight, XMFLOAT3(0.0f, 5.0f, -20.0f), XM_PIDIV4));
	cameras.push_back(std::make_shared<Camera>((float)this->windowWidth / this->windowHeight, XMFLOAT3(0.0f, 2.0f, -3.0f), XM_PIDIV2));
	activeCam = 0;

	directionalLight1 = {};
	directionalLight1.Type = 0;
	directionalLight1.Direction = XMFLOAT3(0, -1, 1);
	directionalLight1.Color = XMFLOAT3(1, 1, 1);
	directionalLight1.Intensity = 0.8f;

	CreateShadowMap();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	setupPP();
	blur = 0;
	ssaoRadius = 1.0f;
	ssaoSamples = 64;
}

void Game::CreateShadowMap()
{
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = windowWidth;
	shadowDesc.Height = windowHeight;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	//depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	//SRV for shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	XMMATRIX lightView = XMMatrixLookToLH(
		XMVectorSet(-directionalLight1.Direction.x * 20, -directionalLight1.Direction.y * 20, -directionalLight1.Direction.z * 20, 0),
		XMVectorSet(directionalLight1.Direction.x, directionalLight1.Direction.y, directionalLight1.Direction.z, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	float lightProjectionSize = 15.0f;
	XMMATRIX lightProj = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProj);
}

void Game::setupPP()
{

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture;
	device->CreateTexture2D(&textureDesc, 0, colorTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ambientTexture;
	device->CreateTexture2D(&textureDesc, 0, ambientTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> SSAOTexture;
	device->CreateTexture2D(&textureDesc, 0, SSAOTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> blurredTexture;
	device->CreateTexture2D(&textureDesc, 0, blurredTexture.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11Texture2D> normalsTexture;
	device->CreateTexture2D(&textureDesc, 0, normalsTexture.GetAddressOf());
	
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
	device->CreateTexture2D(&textureDesc, 0, depthTexture.GetAddressOf());
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());

	device->CreateRenderTargetView(colorTexture.Get(), &rtvDesc, colorRTV.ReleaseAndGetAddressOf());
	device->CreateRenderTargetView(ambientTexture.Get(), &rtvDesc, ambientRTV.ReleaseAndGetAddressOf());
	device->CreateRenderTargetView(SSAOTexture.Get(), &rtvDesc, SSAORTV.ReleaseAndGetAddressOf());
	device->CreateRenderTargetView(blurredTexture.Get(), &rtvDesc, blurredSSAORTV.ReleaseAndGetAddressOf());
	device->CreateRenderTargetView(normalsTexture.Get(), &rtvDesc, normalsRTV.ReleaseAndGetAddressOf());

	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	device->CreateRenderTargetView(depthTexture.Get(), &rtvDesc, depthRTV.ReleaseAndGetAddressOf());

	device->CreateShaderResourceView(colorTexture.Get(), 0, colorSRV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(ambientTexture.Get(), 0, ambientSRV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(SSAOTexture.Get(), 0, SSAOSRV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(blurredTexture.Get(), 0, blurredSSAOSRV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(normalsTexture.Get(), 0, normalsSRV.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(depthTexture.Get(), 0, depthSRV.ReleaseAndGetAddressOf());

	// Create a random texture for SSAO
	const int textureSize = 4;
	const int totalPixels = textureSize * textureSize;
	XMFLOAT4 randomPixels[totalPixels] = {};
	for (int i = 0; i < totalPixels; i++)
	{
		XMVECTOR randomVec = XMVectorSet(RandomRange(-1, 1), RandomRange(-1, 1), 0, 0);
		XMStoreFloat4(&randomPixels[i], XMVector3Normalize(randomVec));
	}
	
	D3D11_TEXTURE2D_DESC randomDesc = {};
	randomDesc.Width = textureSize;
	randomDesc.Height = textureSize;
	randomDesc.MipLevels = 1;
	randomDesc.ArraySize = 1;
	randomDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // Format for storing XMFLOAT4
	randomDesc.SampleDesc.Count = 1;
	randomDesc.Usage = D3D11_USAGE_DEFAULT; // Default usage for GPU access
	randomDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // Bind as shader resource
	randomDesc.CPUAccessFlags = 0;
	randomDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> randomTexture;
	D3D11_SUBRESOURCE_DATA texData = {};
	texData.pSysMem = randomPixels;
	texData.SysMemPitch = textureSize * sizeof(XMFLOAT4);
	device->CreateTexture2D(&randomDesc, &texData, randomTexture.GetAddressOf());
	

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = randomDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(randomTexture.Get(), 0, randomTextureSRV.GetAddressOf());

	// Set up the array of ssao offset vectors (count must match shader!)
	for (int i = 0; i < 64; i++)
	{
		// Offsets should be in hemisphere ([-1,1], [-1,1], [0,1])
		// Note: Must be stored as float4’s due to cbuffer data packing!
		ssaoOffsets[i] = XMFLOAT4(
			(float)rand() / RAND_MAX * 2 - 1, // -1 to 1
			(float)rand() / RAND_MAX * 2 - 1, // -1 to 1
			(float)rand() / RAND_MAX, // 0 to 1
			0);
		XMVECTOR offset = XMVector3Normalize(XMLoadFloat4(&ssaoOffsets[i]));
		// Scale over the array, such that more of the values are
		// closer to the minimum than the maximum (see image -->)
		float scale = (float)i / 64;
		XMVECTOR acceleratedScale = XMVectorLerp(
			XMVectorSet(0.1f, 0.1f, 0.1f, 1),
			XMVectorSet(1, 1, 1, 1),
			scale * scale);
		XMStoreFloat4(&ssaoOffsets[i], offset * acceleratedScale);
	}

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVertexShader.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPixelShader.cso").c_str());
	shadowVShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"ShadowVShader.cso").c_str());
	ppVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"FullscreenVS.cso").c_str());
	blurPPPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PPPixelShader.cso").c_str());
	ppssaoPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SSAOPixelShader.cso").c_str());
	ppssaoblurPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"BlurSSAOPShader.cso").c_str());
	combinePS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"CombineShader.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	
	//bronze
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(), 0, bronze_albedo.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(), 0, bronze_metal.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(), 0, bronze_normals.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(), 0, bronze_roughness.GetAddressOf());

	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(), 0, cobblestone_albedo.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str(), 0, cobblestone_metal.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(), 0, cobblestone_normals.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(), 0, cobblestone_roughness.GetAddressOf());

	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/paint_albedo.png").c_str(), 0, paint_albedo.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/paint_metal.png").c_str(), 0, paint_metal.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/paint_normals.png").c_str(), 0, paint_normals.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/paint_roughness.png").c_str(), 0, paint_roughness.GetAddressOf());

	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(), 0, scratched_albedo.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(), 0, scratched_metal.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(), 0, scratched_normals.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(), 0, scratched_roughness.GetAddressOf());

	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(), 0, wood_albedo.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_metal.png").c_str(), 0, wood_metal.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_normals.png").c_str(), 0, wood_normals.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(), 0, wood_roughness.GetAddressOf());


	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 8;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, sampler.GetAddressOf());

	
	

	bronzeMat = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f);
	cobblestoneMat = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f);
	paintMat = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f);
	scratchedMat = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f);
	woodMat = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.15f);

	bronzeMat->AddTextureSRV("Albedo", bronze_albedo);
	bronzeMat->AddTextureSRV("NormalMap", bronze_normals);
	bronzeMat->AddTextureSRV("RoughnessMap", bronze_roughness);
	bronzeMat->AddTextureSRV("MetalnessMap", bronze_metal);
	bronzeMat->AddSampler("BasicSampler", sampler);

	cobblestoneMat->AddTextureSRV("Albedo", cobblestone_albedo);
	cobblestoneMat->AddTextureSRV("NormalMap", cobblestone_normals);
	cobblestoneMat->AddTextureSRV("RoughnessMap", cobblestone_roughness);
	cobblestoneMat->AddTextureSRV("MetalnessMap", cobblestone_metal);
	cobblestoneMat->AddSampler("BasicSampler", sampler);

	paintMat->AddTextureSRV("Albedo", paint_albedo);
	paintMat->AddTextureSRV("NormalMap", paint_normals);
	paintMat->AddTextureSRV("RoughnessMap", paint_roughness);
	paintMat->AddTextureSRV("MetalnessMap", paint_metal);
	paintMat->AddSampler("BasicSampler", sampler);

	scratchedMat->AddTextureSRV("Albedo", scratched_albedo);
	scratchedMat->AddTextureSRV("NormalMap", scratched_normals);
	scratchedMat->AddTextureSRV("RoughnessMap", scratched_roughness);
	scratchedMat->AddTextureSRV("MetalnessMap", scratched_metal);
	scratchedMat->AddSampler("BasicSampler", sampler);

	woodMat->AddTextureSRV("Albedo", wood_albedo);
	woodMat->AddTextureSRV("NormalMap", wood_normals);
	woodMat->AddTextureSRV("RoughnessMap", wood_roughness);
	woodMat->AddTextureSRV("MetalnessMap", wood_metal);
	woodMat->AddSampler("BasicSampler", sampler);

	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/objStar.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));

	gameEntities.push_back(std::make_shared<gameEntity>(meshes[4], woodMat)); //floor
	gameEntities.push_back(std::make_shared<gameEntity>(meshes[2], scratchedMat)); 
	gameEntities.push_back(std::make_shared<gameEntity>(meshes[6], paintMat)); 
	gameEntities.push_back(std::make_shared<gameEntity>(meshes[0], cobblestoneMat)); 
	gameEntities.push_back(std::make_shared<gameEntity>(meshes[1], bronzeMat)); 

	gameEntities[0]->GetTransform().SetPosition(0.0f, -1.5f, 0.0f);
	gameEntities[0]->GetTransform().SetScale(10.0f, 1.0f, 10.0f);
	gameEntities[1]->GetTransform().SetPosition(-2.0f, 0.0f, 0.0f);
	gameEntities[2]->GetTransform().SetPosition(6.0f, 0.0f, 0.0f);
	gameEntities[4]->GetTransform().SetPosition(2.0f, 0.0f, 0.0f);
	gameEntities[3]->GetTransform().SetPosition(-6.0f, 0.0f, 0.0f); //cube

}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for(int i=0; i<cameras.size(); i++)
		cameras[i]->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	ppRTV.Reset();
	ppSRV.Reset();
	colorRTV.Reset();
	colorSRV.Reset();	
	ambientRTV.Reset();
	ambientSRV.Reset();	
	normalsRTV.Reset();
	normalsSRV.Reset();
	depthRTV.Reset();
	depthSRV.Reset();
	SSAORTV.Reset();
	SSAOSRV.Reset();	
	blurredSSAORTV.Reset();
	blurredSSAOSRV.Reset();
	randomTextureSRV.Reset();

	setupPP();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	//Reset frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	
	gameEntities[3]->GetTransform().Rotate(0.0f, 0.5f * deltaTime,0.0f); 
	gameEntities[1]->GetTransform().Rotate(0.0f, 0.5f * deltaTime,0.0f); 
	
	

	cameras[activeCam]->Update(deltaTime);

	ImGui::ShowDemoWindow();

	ImGui::Begin("Homework Window");
	if (ImGui::TreeNode("Basic Info"))
	{
		ImGui::Text("Frame Rate: %d", ImGui::GetIO().Framerate);
		ImGui::Text("Window Width: %d", this->windowWidth);
		ImGui::Text("Window Height %d", this->windowHeight);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Entities"))
	{
		for (int i = 0; i < 5; i++)
		{
			if (ImGui::TreeNode((void*)(intptr_t)i, "Entity %d", i))
			{
				DirectX::XMFLOAT3 pos = gameEntities[i]->GetTransform().GetPosition();
				DirectX::XMFLOAT3 rot = gameEntities[i]->GetTransform().GetPitchYawRoll();
				DirectX::XMFLOAT3 sca = gameEntities[i]->GetTransform().GetScale();
				if (ImGui::DragFloat3("Position: ", &pos.x))
					gameEntities[i]->GetTransform().SetPosition(pos);
				if (ImGui::DragFloat3("Rotation: ", &rot.x))
					gameEntities[i]->GetTransform().SetRotation(rot);
				if (ImGui::DragFloat3("Scale: ", &sca.x))
					gameEntities[i]->GetTransform().SetScale(sca);
				
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Cameras"))
	{
		ImGui::RadioButton("Camera 1", &activeCam, 0);
		ImGui::RadioButton("Camera 2", &activeCam, 1);
		ImGui::Text("Active Camera");
		DirectX::XMFLOAT3 camPos = cameras[activeCam]->GetTransform()->GetPosition();
		float fov = cameras[activeCam]->GetFOV();
		ImGui::DragFloat3("Position: ", &camPos.x);
		ImGui::DragFloat("Fov: ", &fov);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lights"))
	{
		if (ImGui::TreeNode("Dir Light 1"))
		{
			ImGui::ColorEdit3("color", &directionalLight1.Color.x);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("SRVS"))
	{
		ImGui::Text("Shadow");
		ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Color");
		ImGui::Image(colorSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Ambient");
		ImGui::Image(ambientSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Normals");
		ImGui::Image(normalsSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Depth");
		ImGui::Image(depthSRV.Get(), ImVec2(512, 512));
		ImGui::Text("SSAO");
		ImGui::Image(SSAOSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Blurred SSAO");
		ImGui::Image(blurredSSAOSRV.Get(), ImVec2(512, 512));
		ImGui::Text("Post Process Blur");
		ImGui::Image(ppSRV.Get(), ImVec2(512, 512));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Post Processing"))
	{
		ImGui::SliderInt("Blur ", &blur, 0, 10);
		ImGui::SliderFloat("SSAO Radius", &ssaoRadius, 0.0f, 1.0f);
		ImGui::TreePop();
	}


	ImGui::End();

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();


}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);
		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		//Pre-render 
		
	}
	RenderShadowMap();
	
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // ClearColor
	
	context->ClearRenderTargetView(colorRTV.Get(),   clearColor);
	context->ClearRenderTargetView(ambientRTV.Get(), clearColor);
	context->ClearRenderTargetView(normalsRTV.Get(), clearColor);
	context->ClearRenderTargetView(depthRTV.Get(),   clearColor);
	context->ClearRenderTargetView(SSAORTV.Get(),   clearColor);
	context->ClearRenderTargetView(blurredSSAORTV.Get(),   clearColor);
	context->ClearRenderTargetView(ppRTV.Get(),   clearColor);

	ID3D11RenderTargetView* renderTargets[4] = {};
	renderTargets[0] = colorRTV.Get();
	renderTargets[1] = ambientRTV.Get();
	renderTargets[2] = normalsRTV.Get();
	renderTargets[3] = depthRTV.Get();
	context->OMSetRenderTargets(4, renderTargets, depthBufferDSV.Get());

	for (int i = 0; i < gameEntities.size(); i++)
	{
		std::shared_ptr<SimpleVertexShader> vs = gameEntities[i]->getMaterial()->getVertexShader();
		vs->SetMatrix4x4("lightView", lightViewMatrix);
		vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);
		
		std::shared_ptr<SimplePixelShader> ps = gameEntities[i]->getMaterial()->getPixelShader();
		ps->SetFloat3("ambient", ambientColor);
		ps->SetData("directionalLight1", &directionalLight1, sizeof(Light));
		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);
		gameEntities[i]->DrawEntity(context, cameras[activeCam]);
	}

	skyBox->Draw(context, cameras[activeCam], device);

	context->OMSetRenderTargets(1, SSAORTV.GetAddressOf(), depthBufferDSV.Get());

	XMFLOAT4X4 camProj = cameras[activeCam]->GetProjectionMatrix();
	XMMATRIX proj = XMLoadFloat4x4(&camProj);
	XMFLOAT4X4 invProj;
	XMStoreFloat4x4(&invProj, DirectX::XMMatrixInverse(0, proj));

	ppVS->SetShader();
	ppssaoPS->SetShader();
	ppssaoPS->SetMatrix4x4("viewMatrix", cameras[activeCam]->GetViewMatrix());
	ppssaoPS->SetMatrix4x4("projectionMatrix", cameras[activeCam]->GetProjectionMatrix());
	ppssaoPS->SetMatrix4x4("invProjMatrix", invProj);
	ppssaoPS->SetData("offsets", (void*)(&ssaoOffsets[0]), sizeof(XMFLOAT4) * 64);
	ppssaoPS->SetFloat("ssaoRadius", ssaoRadius);
	ppssaoPS->SetInt("ssaoSamples", ssaoSamples);
	ppssaoPS->SetFloat2("randomTextureScreenScale", XMFLOAT2(windowWidth / 4.0, windowHeight / 4.0));
	ppssaoPS->SetShaderResourceView("Normals", normalsSRV.Get());
	ppssaoPS->SetShaderResourceView("Depths", depthSRV.Get());
	ppssaoPS->SetShaderResourceView("Random", randomTextureSRV.Get());
	ppssaoPS->SetSamplerState("BasicSampler", sampler.Get());
	ppssaoPS->SetSamplerState("ClampSampler", ppSampler.Get());
	ppssaoPS->CopyAllBufferData();
	context->Draw(3, 0);

	context->OMSetRenderTargets(1, blurredSSAORTV.GetAddressOf(), 0);
	ppVS->SetShader();
	ppssaoblurPS->SetShader();
	ppssaoblurPS->SetFloat2("pixelSize", XMFLOAT2(1.0/windowWidth, 1.0/windowHeight));
	ppssaoblurPS->SetShaderResourceView("SSAO", SSAOSRV.Get());
	ppssaoblurPS->SetSamplerState("ClampSampler", ppSampler.Get());
	ppssaoblurPS->CopyAllBufferData();
	context->Draw(3, 0);

	context->ClearRenderTargetView(ppRTV.Get(), clearColor);
	context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), 0);
	ppVS->SetShader();
	combinePS->SetShader();
	combinePS->SetShaderResourceView("SceneColorsNoAmbient", colorSRV.Get());
	combinePS->SetShaderResourceView("Ambient", ambientSRV.Get());
	combinePS->SetShaderResourceView("SSAOBlur", blurredSSAOSRV.Get());
	combinePS->SetSamplerState("BasicSampler", sampler.Get());
	context->Draw(3, 0);


	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);
	// Activate shaders and bind resources
	// Also set any required cbuffer data (not shown)
	ppVS->SetShader();
	blurPPPS->SetShader();
	blurPPPS->SetInt("blurRadius", blur);
	blurPPPS->SetFloat("pixelWidth", 1.0f / this->windowWidth);
	blurPPPS->SetFloat("pixelHeight", 1.0f / this->windowHeight);
	blurPPPS->SetShaderResourceView("Pixels", ppSRV.Get());
	blurPPPS->SetSamplerState("ClampSampler", ppSampler.Get());
	blurPPPS->CopyAllBufferData();
	context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)


	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());

		ID3D11ShaderResourceView* nullSRVs[128] = {};
		context->PSSetShaderResources(0, 128, nullSRVs);
	}
}

//code by Chris Cascioli
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::setCubeSRV(const wchar_t* right,
	const wchar_t* left, const wchar_t* up, 
	const wchar_t* down, const wchar_t* front, 
	const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first texture
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1;            // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;        // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());
	return cubeSRV;
}

void Game::RenderShadowMap() 
{
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	context->PSSetShader(0, 0, 0);

	context->RSSetState(shadowRasterizer.Get());

	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	shadowVShader->SetShader();
	shadowVShader->SetMatrix4x4("view", lightViewMatrix);
	shadowVShader->SetMatrix4x4("projection", lightProjectionMatrix);

	for (auto& e : gameEntities) {
		shadowVShader->SetMatrix4x4("world", e->GetTransform().GetWorldMatrix());
		shadowVShader->CopyAllBufferData();

		e->GetMesh()->Draw();
	}
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetState(0);
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		backBufferRTV.GetAddressOf(),
		depthBufferDSV.Get()
	);
}

