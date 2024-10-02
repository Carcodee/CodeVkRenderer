﻿//
// Created by carlo on 2024-09-24.
//

#ifndef UTILVK_HPP
#define UTILVK_HPP

namespace ENGINE
{
     static SurfaceDetails GetSurfaceDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surfaceKHR)
     {
         SurfaceDetails surfaceDetails;
         surfaceDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surfaceKHR);
         surfaceDetails.formats = physicalDevice.getSurfaceFormatsKHR(surfaceKHR);
         surfaceDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surfaceKHR);

         return surfaceDetails;
     }   
	static uint32_t FindMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t suitableIndices, vk::MemoryPropertyFlags memoryVisibility)
	{
		vk::PhysicalDeviceMemoryProperties availableMemProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < availableMemProperties.memoryTypeCount; i++)
		{
			if ((suitableIndices & (1 << i)) && (availableMemProperties.memoryTypes[i].propertyFlags & memoryVisibility)
				== memoryVisibility)
			{
				return i;
			}
		}
		return uint32_t(-1);
	}

    static std::vector<uint32_t> GetByteCode(const std::string filepath)
    {
     	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

     	if (!file.is_open())
     		throw std::runtime_error("failed to open file!");

     	size_t fileSize = (size_t)file.tellg();
     	std::vector<uint32_t> bytecode(fileSize / sizeof(uint32_t));

     	file.seekg(0);
     	file.read((char*)bytecode.data(), bytecode.size() * sizeof(uint32_t));
     	file.close();
     	
		std::cout << "Read SPIR-V file of size: " << fileSize << " bytes." << std::endl;
     	
     	return bytecode;
    }
    
}


#endif //UTILVK_HPP
