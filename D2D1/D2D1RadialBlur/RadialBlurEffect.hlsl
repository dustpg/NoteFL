// 2D纹理 第一个输入储存在t0
Texture2D InputTexture : register(t0);
// 采样器状态 第一个储存在s0
SamplerState InputSampler : register(s0);

// 常量缓存 (b0)
cbuffer RadialBlurProperties : register(b0) {
    float2  center      : packoffset(c0);
    float   magnitude   : packoffset(c0.z);
    float   samples     : packoffset(c0.w);
};

// Shader入口
float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 sceneSpaceOutput : SCENE_POSITION,
    float4 texelSpaceInput0 : TEXCOORD0
    ) : SV_Target {
    // 初始化
    float4 color = 0;
    // 遍历每个采样点
    for(float i = 0; i < samples; ++i)  {
        // 当前采样进度
        float rate = i / (samples - 1.0);
        // 计算缩放位置比例
        float scale = 1.0f + magnitude * rate;
        // 计算采样地点
        float2 pos = center + (texelSpaceInput0.xy - center) * scale;
        // 添加采样数据
        color += InputTexture.Sample(InputSampler, pos);
    }
    // 平均
    return color / samples;
}
