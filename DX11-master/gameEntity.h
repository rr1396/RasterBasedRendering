#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>
#include "Camera.h"
#include "material.h"

class gameEntity
{
private:
	Transform transformObj;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

public:
	gameEntity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);
	~gameEntity();

	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();
	std::shared_ptr<Material> getMaterial();
	void setMaterial(std::shared_ptr<Material> _material);

	void DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera);
};

