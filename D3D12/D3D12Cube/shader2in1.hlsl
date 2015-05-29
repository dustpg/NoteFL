// C Buffer 0 : 储存转换矩阵
cbuffer MatrixBuffer : register (b0) {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// VS 输入
struct VertexInputType {
    float4 position     : POSITION;
    float4 color        : COLOR;
};

// VS 输出
struct PixelInputType {
    float4 position     : SV_POSITION;
    float4 color        : COLOR;
};

// 处理
PixelInputType ColorVertexShader(VertexInputType input) {
    PixelInputType output;
    // 坐标转换
    output.position = mul(float4(input.position.xyz, 1), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    // 直接输出
    output.color = input.color;

    return output;
}

// 像素着色器处理
float4 ColorPixelShader(PixelInputType input) : SV_TARGET {
    return input.color;
}
