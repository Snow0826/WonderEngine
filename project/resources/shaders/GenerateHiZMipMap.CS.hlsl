Texture2D<float> srcMip : register(t0);
RWTexture2D<float> dstMip : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = DTid.xy * 2;
    float d0 = srcMip.Load(int3(uv, 0));
    float d1 = srcMip.Load(int3(uv + float2(1, 0), 0));
    float d2 = srcMip.Load(int3(uv + float2(0, 1), 0));
    float d3 = srcMip.Load(int3(uv + float2(1, 1), 0));
    dstMip[DTid.xy] = max(max(d0, d1), max(d2, d3));
}