#include "Object3d.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Material
{
    float4 color;
    uint enableLighting;
    uint enableFlipV;
    float4x4 uvTransform;
    float shininess;
    float3 specular;
    float environmentCoefficient;
};

struct Camera
{
    float3 worldPosition;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<Camera> gCamera : register(b1);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);

cbuffer TextureData : register(b3)
{
    uint textureHandle;
    uint enableMipmaps;
};

cbuffer LightData : register(b4)
{
    uint gPointLightCount;
    uint gSpotLightCount;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};

StructuredBuffer<PointLight> gPointLight : register(t0);
StructuredBuffer<SpotLight> gSpotLight : register(t1);
TextureCube<float4> gEnvironmentMap : register(t2);
Texture2D<float4> gTextures[] : register(t3);
SamplerState gSampler : register(s0);
SamplerState gSamplerMip0 : register(s1);

float CalculateAttenuation(float3 lightPosition, float3 fragmentPosition, float lightDistance, float decay);

float3 CalculateDirection(float3 lightPosition, float3 fragmentPosition);

float3 ApplyLightCommon(float3 lightDirection, VertexShaderOutput input, float4 textureColor);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float2 texcoord = input.texcoord;
    if (gMaterial.enableFlipV != 0)
    {
        texcoord.y = 1.0f - texcoord.y; // Flip the y-coordinate for texture sampling
    }
    float4 transformedUV = mul(float4(texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor;
    if (enableMipmaps != 0)
    {
        textureColor = gTextures[textureHandle].Sample(gSampler, transformedUV.xy);
    }
    else
    {
        textureColor = gTextures[textureHandle].Sample(gSamplerMip0, transformedUV.xy);
    }
    
    if (gMaterial.enableLighting != 0)
    {
        output.color.rgb = ApplyLightCommon(gDirectionalLight.direction, input, textureColor) * gDirectionalLight.color.rgb * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
        
        for (uint i = 0; i < gPointLightCount; ++i)
        {
            PointLight light = gPointLight[i];
            float3 direction = CalculateDirection(light.position, input.worldPosition);
            float attenuation = CalculateAttenuation(light.position, input.worldPosition, light.radius, light.decay);
            output.color.rgb += ApplyLightCommon(direction, input, textureColor) * light.color.rgb * light.intensity * attenuation;
        }
        
        for (uint j = 0; j < gSpotLightCount; ++j)
        {
            SpotLight light = gSpotLight[j];
            float3 direction = CalculateDirection(light.position, input.worldPosition);
            float attenuation = CalculateAttenuation(light.position, input.worldPosition, light.distance, light.decay);
            float cosAngle = dot(direction, light.direction);
            float falloff = saturate((cosAngle - light.cosAngle) / (light.cosFalloffStart - light.cosAngle));
            output.color.rgb += ApplyLightCommon(direction, input, textureColor) * light.color.rgb * light.intensity * attenuation * falloff;
        }
        
        float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
        float4 environmentColor = gEnvironmentMap.Sample(gSampler, reflectedVector);
        output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}

float CalculateAttenuation(float3 lightPosition, float3 fragmentPosition, float lightDistance, float decay)
{
    float distance = length(lightPosition - fragmentPosition);
    float attenuation = pow(saturate(-distance / lightDistance + 1.0f), decay);
    return attenuation;
}

float3 CalculateDirection(float3 lightPosition, float3 fragmentPosition)
{
    return normalize(fragmentPosition - lightPosition);
}

float3 ApplyLightCommon(float3 lightDirection, VertexShaderOutput input, float4 textureColor)
{
    float NdotL = dot(normalize(input.normal), -lightDirection);
    //float cos = saturate(NdotL); // Lambert
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f); // Half Lambert
    float3 viewDir = normalize(gCamera.worldPosition - input.worldPosition);
    float3 halfDir = normalize(-lightDirection + viewDir);
    float3 NdotH = dot(normalize(input.normal), halfDir);
    float3 diffuse = gMaterial.color.rgb * textureColor.rgb * cos;
    float3 specular = pow(saturate(NdotH), gMaterial.shininess) * gMaterial.specular;
    return diffuse + specular;
}