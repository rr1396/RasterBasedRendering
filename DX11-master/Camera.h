#pragma once
#include "Input.h"
#include "Transform.h"

class Camera
{
private:
	Transform* transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	float fov;
	float nearZ;
	float farZ;
	float movementSpeed;
	float mouseLookSpeed;
	bool isPerspective;

public:
	//constructor
	Camera(float aspectRatio, DirectX::XMFLOAT3 initalPos, float _fov);

	//getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	Transform* GetTransform();
	float GetFOV();

	//methods
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);




};

