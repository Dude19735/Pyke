#pragma once

#include <unordered_map>

#include "../Defines.h"
#include "../external/tabulate/single_include/tabulate/tabulate.hpp"
#include "Vk_Device.hpp"

namespace VK4 {

    class Vk_DeviceQueue {
    public:
        Vk_DeviceQueue(Vk_Device* device)
        :
        _device(device)
        {
            queryAvailableQueueTypes();
        }
    private:
        struct QueueFamily {
            uint32_t queueFamilyCount;
            std::vector<VkQueueFamilyProperties> queueFamilyProperties;        
        };

        Vk_Device* _device;
        std::unordered_map<int, VkQueue> _queues;
        // {PhysicalDeviceIndex : {QueueIndex : {QueueProps_1 ... QueueProps_N}}}
        std::unordered_map<int, QueueFamily> _deviceProps;

        void queryAvailableQueueTypes(){
            const auto& physicalDevices = _device->vk_allPhysicalDevices();

            for(const auto& pd : physicalDevices){
                _deviceProps.insert({pd.physicalDeviceIndex, {}});
            }

            for(const auto& pd : physicalDevices){
                // query device queue indices for graphics and present family
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(
                    pd.physicalDevice,
                    &queueFamilyCount,
                    nullptr
                );

                _deviceProps.insert({
                    pd.physicalDeviceIndex, 
                    QueueFamily{
                        .queueFamilyCount=queueFamilyCount,
                        .queueFamilyProperties=std::vector<VkQueueFamilyProperties>(queueFamilyCount)
                    }
                });

                vkGetPhysicalDeviceQueueFamilyProperties(
                    pd.physicalDevice,
                    &queueFamilyCount,
                    _deviceProps.at(pd.physicalDeviceIndex).queueFamilyProperties.data()
                );
            }
        }

        std::string queueFamily2String(){
            const auto& physicalDevices = _device->vk_allPhysicalDevices();

            tabulate::Table table;
            table.add_row({"GPU", "DeviceIndex", "VkQueueFamilyProperties"});
            for(const auto& pd : physicalDevices){

            }
        }
    };
}