#include "gameEntity.h"
#include "BufferStructs.h"
#include "Vertex.h"

gameEntity::gameEntity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material)
{
	mesh = _mesh;
	transformObj = Transform();
	material = _material;
}

gameEntity::~gameEntity()
{
	
}

std::shared_ptr<Mesh> gameEntity::GetMesh()
{
	return mesh;
}

Transform& gameEntity::GetTransform()
{
	return transformObj;
}

std::shared_ptr<Material> gameEntity::getMaterial()
{
	return material;
}

void gameEntity::setMaterial(std::shared_ptr<Material> _material)
{
	material = _material;
}

void gameEntity::DrawEntity(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	material->setShaders(transformObj.GetWorldMatrix(), camera->GetViewMatrix(), camera->GetProjectionMatrix(), transformObj.GetWorldInverseTransposeMatrix(), camera->GetTransform()->GetPosition());

	mesh->Draw();
}
