#pragma once
#include <DirectXMath.h>


class Transform
{
private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
public:
	Transform();
	//setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 _position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 _rotation);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 _scale);

	//getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	//Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 _rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 _scale);

	void MoveRelative(float x, float y, float z);
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();

	void UpdateMatrices();
};

