﻿#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 textCoord;

void main() {
    outColor = vec4(textCoord, 0.0f, 1.0f);
}