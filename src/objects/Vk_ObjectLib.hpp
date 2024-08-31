#pragma once

#include <vector>

#include "../Defines.h"
#include "../application/Vk_Device.hpp"
#include "Vk_Structures.hpp"
#include "../buffers/Vk_DataBuffer.hpp"

namespace VK4 {
    class Vk_ObjectLib {
    public:
        template<typename T_StructureType, typename T_DataType> 
        static void vk_updateBuffer(
            Vk_Device* device,
            Vk_ObjUpdate updateMode,
            Vk_DataBuffer<T_StructureType>* buffer,
			const T_DataType* data,
            size_t dataSize,
			size_t newFrom,
			size_t newTo
		){
            int inDim = 1;
            // for index data, we don't have an innerDimensionLen because index_data is
            // an unsigned int
            if constexpr(!std::is_same_v<T_StructureType, index_type>){
                inDim = T_StructureType::innerDimensionLen();
            }

            size_t len = dataSize;
            if constexpr(!std::is_same_v<T_StructureType, T_DataType>){
                // In case T_StructureType and T_DataType are not the same, this part is important
                // (same example as below)
                len = static_cast<size_t>(dataSize / inDim);

                // if the type of the structured data is not the same as the update data
                // (for example Vk_Vertex_P and point_type), then the size of the update data
                // must match the structure length of the structured data (for example
                // for Vk_Vertex_P that is 3, so dataSize must be a multiple of 3)
                if(!(dataSize % inDim == 0)){
                    Vk_Logger::RuntimeError(typeid(NoneObj), "DataType size must be a multiple of {0} but is {1}", inDim, dataSize);
                }
            }

			const T_StructureType* d = reinterpret_cast<const T_StructureType*>(data);

            // so some error catching
			if(newFrom >= len){
				Vk_Logger::RuntimeError(typeid(NoneObj), "'newFrom' must be smaller than data.size() but newFrom={0} and data.size()={0}", newFrom, len);
			}
            if(newTo >= len){
				Vk_Logger::RuntimeError(typeid(NoneObj), "'newTo' must be smaller or equal data.size() but newTo={0} and data.size()={0}", newTo, len);
			}
            // allow 0 for simplicity to update everything
            if(newTo == 0) newTo = len;
			
            // call the generic buffer update method
			buffer->vk_update(d, len, newFrom, newTo);

            // Most of the time, after an update we want to rebuild the Vulkan rendering commands.
            // If we have multiple updates in a row, we can use 'Deferred' here to first update everything
            // and only use 'Promptly' for the last update to rebuild the rendering commands and cause the
            // window to emit a PAINT event.
            // Anyways, steering things is what the bridge is for
            if(updateMode == Vk_ObjUpdate::Promptly){
                device->bridge.rebuildFrames();
            }
		}
    };
}