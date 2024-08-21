#pragma once

#include <vector>
#include <string>

#include "../Defines.h"
// #include "../Vk_Logger.hpp"
#include "Vk_Device.hpp"

namespace VK4 {
	class Vk_SamplingResolution {
	public:
		Vk_SamplingResolution(Vk_Device* const device, VkSampleCountFlagBits desiredSamplingResolution)
		:
			_device(device),
			_samplingResolution(VK_SAMPLE_COUNT_1_BIT)
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Sampling Resolution"));

			VkSampleCountFlagBits maxRes = 
				_device->vk_maxUsableSampleCount();

			if (desiredSamplingResolution == VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM) {
				desiredSamplingResolution = maxRes;
			}
			else if (desiredSamplingResolution > maxRes) {
				Vk_Logger::Warn(typeid(this), 
					std::string("Desired over-sampling resolution of ")
						+ std::to_string(static_cast<int>(desiredSamplingResolution))
						+ " is higher than device max of "
						+ std::to_string(static_cast<int>(maxRes))
						+ "\nSet resolution to device max!"
				);

				desiredSamplingResolution = maxRes;
			}
			else {
				bool valid = false;
				switch (desiredSamplingResolution) {
				case(VK_SAMPLE_COUNT_1_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_2_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_4_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_8_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_16_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_32_BIT):
					valid = true;
					break;
				case(VK_SAMPLE_COUNT_64_BIT):
					valid = true;
					break;
				default:
					valid = false;
				}

				ASSERT_MSG(valid, "Over-Sampling resolution is invalid!");
			}

			//_samplingResolution = VK_SAMPLE_COUNT_1_BIT;
			_samplingResolution = desiredSamplingResolution;
		}

		~Vk_SamplingResolution(){
			Vk_Logger::Log(typeid(this), GlobalCasters::castDestructorTitle("Destroy Sampling Resolution"));
		}

		VkSampleCountFlagBits vk_samplingResolution() {
			return _samplingResolution;
		}

		bool vk_isMultisampling() {
			return _samplingResolution != VK_SAMPLE_COUNT_1_BIT;
		}

	private:
		Vk_Device* _device;
		VkSampleCountFlagBits _samplingResolution;
	};
}
