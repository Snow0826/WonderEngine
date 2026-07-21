#include "Object3d.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
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

ConstantBuffer<Camera> gCamera : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

cbuffer LightData : register(b2)
{
    uint gPointLightCount;
    uint gSpotLightCount;
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

struct TextureData
{
    uint textureHandle;
    uint enableMipmaps;
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

StructuredBuffer<Material> gMaterial : register(t0);
StructuredBuffer<TextureData> gTextureData : register(t1);
StructuredBuffer<PointLight> gPointLight : register(t2);
StructuredBuffer<SpotLight> gSpotLight : register(t3);
TextureCube<float4> gEnvironmentMap : register(t4);
Texture2D<float4> gTextures[] : register(t5);
SamplerState gSampler : register(s0);
SamplerState gSamplerMip0 : register(s1);

float CalculateAttenuation(float3 lightPosition, float3 fragmentPosition, float lightDistance, float decay);

float3 CalculateDirection(float3 lightPosition, float3 fragmentPosition);

float3 ApplyLightCommon(float3 lightDirection, VertexShaderOutput input, float4 textureColor);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float2 texcoord = input.texcoord;
    uint instanceIndex = input.instanceIndex;
    if (gMaterial[instanceIndex].enableFlipV != 0)
    {
        texcoord.y = 1.0f - texcoord.y; // Flip the y-coordinate for texture sampling
    }
    float4 transformedUV = mul(float4(texcoord, 0.0f, 1.0f), gMaterial[instanceIndex].uvTransform);
    float4 textureColor;
    if (gTextureData[instanceIndex].enableMipmaps != 0)
    {
        textureColor = gTextures[gTextureData[instanceIndex].textureHandle].Sample(gSampler, transformedUV.xy);
    }
    else
    {
        textureColor = gTextures[gTextureData[instanceIndex].textureHandle].Sample(gSamplerMip0, transformedUV.xy);
    }
    
    if (gMaterial[instanceIndex].enableLighting != 0)
    {
        output.color.rgb = ApplyLightCommon(gDirectionalLight.direction, input, textureColor) * gDirectionalLight.color.rgb * gDirectionalLight.intensity;
        output.color.a = gMaterial[instanceIndex].color.a * textureColor.a;
        
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
        output.color.rgb += environmentColor.rgb * gMaterial[instanceIndex].environmentCoefficient;
    }
    else
    {
        output.color = gMaterial[instanceIndex].color * textureColor;
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
    float3 diffuse = gMaterial[input.instanceIndex].color.rgb * textureColor.rgb * cos;
    float3 specular = pow(saturate(NdotH), gMaterial[input.instanceIndex].shininess) * gMaterial[input.instanceIndex].specular;
    return diffuse + specular;
}