struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    nointerpolation uint instanceIndex : INSTANCEINDEX0;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint section : SECTION0; // 0 = side, 1 = top, 2 = bottom
};

struct PerView
{
    float4x4 view;
    float4x4 projection;
};

struct MeshData
{
    uint meshOffset;
};

struct InstanceData
{
    float4x4 world;
    float4x4 worldInverseTranspose;
    float bottomRadius;
    float topRadius;
    float inverseBranchLength;
};