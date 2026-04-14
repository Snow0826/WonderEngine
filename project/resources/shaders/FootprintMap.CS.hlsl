Texture2D<float4> footprintMap : register(t0);
RWStructuredBuffer<int4> color : register(u0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int4 footprintColor = int4(footprintMap.Load(int3(DTid.xy, 0)));
    if (footprintColor.r == 1 &&
        footprintColor.g == 1 &&
        footprintColor.b == 1 &&
        footprintColor.a == 1)
        return;
    InterlockedAdd(color[0].r, footprintColor.r);
    InterlockedAdd(color[0].g, footprintColor.g);
    InterlockedAdd(color[0].b, footprintColor.b);
    InterlockedAdd(color[0].a, footprintColor.a);
}