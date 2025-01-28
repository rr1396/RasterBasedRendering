#include "Transform.h"

Transform::Transform()
{
    position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

    DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(&worldInverseTransposeMatrix, DirectX::XMMatrixIdentity());
}

void Transform::SetPosition(float x, float y, float z)
{
    position = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
    position = _position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
    rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
    rotation = _rotation;
}

void Transform::SetScale(float x, float y, float z)
{
    scale = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
    scale = _scale;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
    return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
    return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
    return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
    UpdateMatrices();
    return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
    UpdateMatrices();
    return worldInverseTransposeMatrix;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
    position.x += x;
    position.y += y;
    position.z += z;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
    rotation.x += pitch;
    rotation.y += yaw;
    rotation.z += roll;
}

void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
    rotation.x += _rotation.x;
    rotation.y += _rotation.y;
    rotation.z += _rotation.z;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
}

void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
    scale.x *= _scale.x;
    scale.y *= _scale.y;
    scale.z *= _scale.z;
}

void Transform::MoveRelative(float x, float y, float z)
{
    DirectX::XMVECTOR direc = DirectX::XMVectorSet(x, y, z, 0.0f);
    DirectX::XMVECTOR rotat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    direc = DirectX::XMVector3Rotate(direc, rotat);
    DirectX::XMVECTOR currentPos = DirectX::XMLoadFloat3(&position);
    direc = DirectX::XMVectorAdd(direc, currentPos);
    DirectX::XMStoreFloat3(&position, direc);
}

DirectX::XMFLOAT3 Transform::GetRight()
{
    DirectX::XMVECTOR direc = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR rotat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    direc = DirectX::XMVector3Rotate(direc, rotat);
    DirectX::XMFLOAT3 right;
    DirectX::XMStoreFloat3(&right, direc);
    return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
    DirectX::XMVECTOR direc = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR rotat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    direc = DirectX::XMVector3Rotate(direc, rotat);
    DirectX::XMFLOAT3 up;
    DirectX::XMStoreFloat3(&up, direc);
    return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
    DirectX::XMVECTOR direc = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR rotat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    direc = DirectX::XMVector3Rotate(direc, rotat);
    DirectX::XMFLOAT3 forward;
    DirectX::XMStoreFloat3(&forward, direc);
    return forward;
}

void Transform::UpdateMatrices()
{
    DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMMATRIX rotat = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    DirectX::XMMATRIX scal = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    DirectX::XMMATRIX world = scal * rotat * trans;

    DirectX::XMStoreFloat4x4(&worldMatrix, world);
    DirectX::XMStoreFloat4x4(&worldInverseTransposeMatrix, DirectX::XMMatrixInverse(0, DirectX::XMMatrixTranspose(world)));
}
