#include "Camera.h"

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 initalPos, float _fov)
{
    transform = new Transform();
    transform->SetPosition(initalPos);
    fov = _fov;
    nearZ = 0.1f;
    farZ = 100.0f;
    movementSpeed = 1.0f;
    mouseLookSpeed = 0.05f;
    UpdateProjectionMatrix(aspectRatio);
    UpdateViewMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
    return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
    return projectionMatrix;
}

Transform* Camera::GetTransform()
{
    return transform;
}

float Camera::GetFOV()
{
    return fov;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    if (isPerspective)
        DirectX::XMStoreFloat4x4(&projectionMatrix, DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ));

}

void Camera::UpdateViewMatrix()
{
    DirectX::XMFLOAT3 worldUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    DirectX::XMFLOAT3 pos = transform->GetPosition();
    DirectX::XMFLOAT3 forward = transform->GetForward();
    DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&forward), DirectX::XMLoadFloat3(&worldUp)));
}

void Camera::Update(float dt)
{
    Input& input = Input::GetInstance();

    if (input.KeyDown('W')) 
    {
        transform->MoveRelative(0.0f,0.0f,movementSpeed*dt);
    }
    if (input.KeyDown('S')) 
    {
        transform->MoveRelative(0.0f, 0.0f, -movementSpeed * dt);
    }
    if (input.KeyDown('A')) 
    {
        transform->MoveRelative(-movementSpeed * dt, 0.0f, 0.0f);
    }
    if (input.KeyDown('D')) 
    {
        transform->MoveRelative(movementSpeed * dt ,0.0f, 0.0f);
    }
    if (input.KeyDown(VK_SPACE)) 
    {
        transform->MoveAbsolute(0.0f, movementSpeed * dt, 0.0f);
    }
    if (input.KeyDown('X'))
    {
        transform->MoveAbsolute(0.0f, -movementSpeed * dt, 0.0f);
    }

    if (input.MouseLeftDown())
    {
        int cursorMovementX = input.GetMouseXDelta();
        int cursorMovementY = input.GetMouseYDelta();

        transform->Rotate(cursorMovementY * mouseLookSpeed, cursorMovementX * mouseLookSpeed, 0.0f);
        if (transform->GetPitchYawRoll().x > DirectX::XM_PIDIV2)
            transform->SetRotation(DirectX::XM_PIDIV2, transform->GetPitchYawRoll().y, transform->GetPitchYawRoll().z);
        if (transform->GetPitchYawRoll().x < -DirectX::XM_PIDIV2)
            transform->SetRotation(-DirectX::XM_PIDIV2, transform->GetPitchYawRoll().y, transform->GetPitchYawRoll().z);
    }
    UpdateViewMatrix();
}

