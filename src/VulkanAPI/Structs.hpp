﻿//
// Created by carlo on 2024-09-22.
//

#ifndef STRUCTS_HPP
namespace ENGINE
{
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamilyIndex;
        uint32_t presentFamilyIndex;
    };
    struct WindowDesc
    {
        HINSTANCE hInstance;
        HWND hWnd;
    };
    
}
#define STRUCTS_HPP

#endif //STRUCTS_HPP
