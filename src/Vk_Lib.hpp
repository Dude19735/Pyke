#pragma once

#include "Defines.h"
#include <iostream>

namespace VK4 {
    class Vk_Lib {
    public:
        static bool replace(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos = str.find(from);
			if (start_pos == std::string::npos)
				return false;
			str.replace(start_pos, from.length(), to);
			return true;
		}

		static double round(double value, int decimals) {
			double f = decimals * 10.0;
			return std::round(value * f) / f;
		}

		static std::string rightCrop(double value) {
			std::string rVal = std::to_string(value);

			// Remove trailing zeroes
			std::string cVal = rVal.substr(0, rVal.find_last_not_of('0') + 1);
			// If the decimal point is now the last character, remove that as well
			if (cVal.find('.') == cVal.size() - 1)
			{
				cVal = cVal.substr(0, cVal.size() - 1);
			}
			return cVal;
		}
    };
}