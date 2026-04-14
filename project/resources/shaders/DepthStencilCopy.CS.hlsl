Texture2D<float> srcMip : register(t0);
RWTexture2D<float> dstMip : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float d = srcMip.Load(int3(DTid.xy, 0));
    dstMip[DTid.xy] = d;
}