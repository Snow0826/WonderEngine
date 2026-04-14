#include "Line.hlsli"

struct Line
{
    float3 start;
    float3 end;
    float4 color;
};

StructuredBuffer<Line> gLine : register(t0);

cbuffer ViewProjection : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

VertexShaderOutput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    float3 position = (vertexId == 0) ? gLine[instanceId].start : gLine[instanceId].end;
    output.position = mul(float4(position, 1.0f), mul(view, projection));
    output.color = gLine[instanceId].color;
    return output;
}