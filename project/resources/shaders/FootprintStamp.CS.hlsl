struct Footprint
{
    float3 worldPosition; // X, Y, Z position of the footprint
    float radius; // Radius of the footprint
    float4 color; // Color of the footprint
};

cbuffer PerDispatch : register(b0)
{
    float2 terrainOriginXZ; // X, Y origin of the terrain chunk
    float2 terrainSizeXZ; // Size of the terrain chunk in X and Y
};

StructuredBuffer<Footprint> footprints : register(t0);
RWTexture2D<float4> footprintMapRW : register(u0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int px = DTid.x;
    int py = DTid.y;
    int index = DTid.z;

    Footprint footprint = footprints[index];
    if (abs(footprint.worldPosition.y) > footprint.radius)
    {
        return;
    }
    
    uint footprintMapWidth, footprintMapHeight;
    footprintMapRW.GetDimensions(footprintMapWidth, footprintMapHeight);
    float u = ((float)px + 0.5f) / (float)footprintMapWidth;
    float v = ((float)py + 0.5f) / (float)footprintMapHeight;
    float worldX = terrainOriginXZ.x + u * terrainSizeXZ.x;
    float worldZ = terrainOriginXZ.y + v * terrainSizeXZ.y;
    footprint.worldPosition.z = -footprint.worldPosition.z;
    footprint.worldPosition.xz += terrainSizeXZ.xy * 0.5f;

    float2 local = float2(worldX, worldZ) - footprint.worldPosition.xz;
    float dist = length(local);
    if (dist > footprint.radius)
    {
        return;
    }
    
    footprintMapRW[int2(px, py)] = footprint.color;
}