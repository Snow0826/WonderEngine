struct VertexShaderInput
{
    float4 position : POSITION0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

cbuffer PerView : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer SoftBodyData : register(b1)
{
    float4x4 world;
    float impactVelocity;
    float hitTime;
    
    float impactScale;
    float maxImpact;
    
    float damping;
    float frequency;
    
    float squashAmount;
    float expandAmount;
    
    float lowerExpandWeight;
    float upperExpandWeight;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float3 position = input.position.xyz;

    //--------------------------------------------------
    // Cubeのローカル範囲
    //--------------------------------------------------

    const float bottomY = -0.5;
    const float topY = 0.5;

    //--------------------------------------------------
    // 0～1の高さ
    //--------------------------------------------------

    float normalizedY = saturate((position.y - bottomY) / (topY - bottomY));

    //--------------------------------------------------
    // 着地衝撃
    //--------------------------------------------------

    float impact = saturate(impactVelocity * impactScale);
    impact = min(impact, maxImpact);

    //--------------------------------------------------
    // 減衰振動
    //--------------------------------------------------

    float oscillation = exp(-damping * hitTime) * cos(frequency * hitTime);

    //--------------------------------------------------
    // Squash
    //--------------------------------------------------

    float squash = impact * squashAmount * oscillation;

    //--------------------------------------------------
    // Y方向を圧縮
    //--------------------------------------------------

    float height = position.y - bottomY;
    height *= 1.0 - squash;
    position.y = bottomY + height;

    //--------------------------------------------------
    // XZ方向へ膨らむ
    //--------------------------------------------------

    float expand = squash * expandAmount;

    // 下側ほど大きく広がる
    float expandWeight = 1.0 - normalizedY;
    position.xz *= 1.0 + expand * expandWeight * lowerExpandWeight;

    //--------------------------------------------------
    // 上側を少し横へ引っ張る
    //--------------------------------------------------

    float upperWeight = normalizedY;
    position.xz *= 1.0 + expand * upperWeight * upperExpandWeight;

    //--------------------------------------------------
    // World Transform
    //--------------------------------------------------

    float4 worldPosition = mul(float4(position, 1.0), world);

    //--------------------------------------------------
    // View
    //--------------------------------------------------

    float4 viewPosition = mul(worldPosition, view);

    //--------------------------------------------------
    // Projection
    //--------------------------------------------------

    output.position = mul(viewPosition, projection);
    return output;
}