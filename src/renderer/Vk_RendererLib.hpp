#pragma once

#include "../Defines.h"

namespace VK4 {
    enum class Vk_SubmitResult {
        Ok,
        Reset
    };

    class Vk_RendererLib {
    public:
        static Vk_SubmitResult checkSubmitResult(const std::type_info& info, const std::string& origin, VkResult result){
            switch(result) {
                case VK_SUCCESS: break;
                case VK_SUBOPTIMAL_KHR:
                    Vk_Logger::Log(info, "{0} subission suboptimal, reset viewer [VK_SUBOPTIMAL_KHR]!", origin);
                    return Vk_SubmitResult::Reset;
                break;
                case VK_ERROR_OUT_OF_DATE_KHR:
                    Vk_Logger::Log(info, "{0} subission out of date, reset viewer [VK_ERROR_OUT_OF_DATE_KHR]!", origin);
                    return Vk_SubmitResult::Reset;
                break;
                    case VK_ERROR_OUT_OF_HOST_MEMORY:
                    Vk_Logger::RuntimeError(info, "{0} subission failed [VK_ERROR_OUT_OF_HOST_MEMORY]!", origin);
                break;
                    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    Vk_Logger::RuntimeError(info, "{0} subission failed [VK_ERROR_OUT_OF_DEVICE_MEMORY]!", origin);
                break;
                case VK_ERROR_DEVICE_LOST:
                    Vk_Logger::RuntimeError(info, "{0} subission failed [VK_ERROR_DEVICE_LOST]!", origin);
                    break;
                case VK_ERROR_SURFACE_LOST_KHR:
                    Vk_Logger::RuntimeError(info, "{0} subission failed [VK_ERROR_SURFACE_LOST_KHR]!", origin);
                    break;
                case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                    Vk_Logger::RuntimeError(info, "{0} subission failed [VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT]!", origin);
                    break;
                default:
                    Vk_Logger::RuntimeError(info, "{0} submission unknown error [{1}]", origin, static_cast<int>(result));

                return Vk_SubmitResult::Ok;
            }
        }
    };
}