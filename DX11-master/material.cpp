#include "material.h"

Material::Material(DirectX::XMFLOAT4 _colorTint, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness)
{
    colorTint = _colorTint;
    vertexShader = _vertexShader;
    pixelShader = _pixelShader;
    roughness = _roughness;
}

std::shared_ptr<SimpleVertexShader> Material::getVertexShader()
{
    return vertexShader;
}

std::shared_ptr<SimplePixelShader> Material::getPixelShader()
{
    return pixelShader;
}

DirectX::XMFLOAT4 Material::getColorTint()
{
    return colorTint;
}

float Material::getRoughness()
{
    return roughness;
}

void Material::setVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader)
{
    vertexShader = _vertexShader;
}

void Material::setPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader)
{
    pixelShader = _pixelShader;
}

void Material::setColorTint(DirectX::XMFLOAT4 _colorTint)
{
    colorTint = _colorTint;
}


void Material::setShaders(DirectX::XMFLOAT4X4 worldMatrix, 
    DirectX::XMFLOAT4X4 viewMatrix, 
    DirectX::XMFLOAT4X4 projectionMatrix, 
    DirectX::XMFLOAT4X4 worldInverseTransposeMatrix,
    DirectX::XMFLOAT3 position)
{
    getVertexShader()->SetShader();
    getPixelShader()->SetShader();

    std::shared_ptr<SimpleVertexShader> vs = getVertexShader();


    vs->SetMatrix4x4("world", worldMatrix);
    vs->SetMatrix4x4("view", viewMatrix);
    vs->SetMatrix4x4("projection", projectionMatrix);
    vs->SetMatrix4x4("worldInverseTranspose", worldInverseTransposeMatrix);

    vs->CopyAllBufferData();

    std::shared_ptr<SimplePixelShader> ps = getPixelShader();

    ps->SetFloat4("colorTint", getColorTint());
    ps->SetFloat("roughness", getRoughness());
    ps->SetFloat3("cameraPos", position);
    for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
    for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second); }

    ps->CopyAllBufferData();
}

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({shaderName, srv});
}

void Material::AddSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers.insert({shaderName, sampler});
}
