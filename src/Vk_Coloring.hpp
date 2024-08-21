#pragma once

#include <string>
#include <vector>

namespace VK4 {
    struct Vk_RGBColor {
		float r, g, b;

        static int innerDimensionLen(){ return 3; }
	};

    namespace RGBA {
        Vk_RGBColor Red { 1.0f, 0.0f, 0.0f };
        Vk_RGBColor Green { 0.0f, 1.0f, 0.0f };
        Vk_RGBColor Blue { 0.0f, 0.0f, 1.0f };
        Vk_RGBColor Black { 0.0f, 0.0f, 0.0f };
        Vk_RGBColor White { 1.0f, 1.0f, 1.0f };
    }

    struct Vk_OklabColor {
		float L, a, b;
	};

    /**
    * Name            FG  BG
    * Black           30  40
    * Red             31  41
    * Green           32  42
    * Yellow          33  43
    * Blue            34  44
    * Magenta         35  45
    * Cyan            36  46
    * White           37  47
    * Bright Black    90  100
    * Bright Red      91  101
    * Bright Green    92  102
    * Bright Yellow   93  103
    * Bright Blue     94  104
    * Bright Magenta  95  105
    * Bright Cyan     96  106
    * Bright White    97  107
    */

    // basic text colors
    static const std::string TE_COL_BLACK =   "\x1B[30m";
    static const std::string TE_COL_RED =     "\x1B[31m";
    static const std::string TE_COL_GREEN =   "\x1B[32m";
    static const std::string TE_COL_YELLOW =  "\x1B[33m";
    static const std::string TE_COL_BLUE =    "\x1B[34m";
    static const std::string TE_COL_MAGENTA = "\x1B[35m";
    static const std::string TE_COL_CYAN =    "\x1B[36m";
    static const std::string TE_COL_WHITE =   "\x1B[37m";

    static const std::string TE_COL_BRIGHT_BLACK =   "\x1B[90m";
    static const std::string TE_COL_BRIGHT_RED =     "\x1B[91m";
    static const std::string TE_COL_BRIGHT_GREEN =   "\x1B[92m";
    static const std::string TE_COL_BRIGHT_YELLOW =  "\x1B[93m";
    static const std::string TE_COL_BRIGHT_BLUE =    "\x1B[94m";
    static const std::string TE_COL_BRIGHT_MAGENTA = "\x1B[95m";
    static const std::string TE_COL_BRIGHT_CYAN =    "\x1B[96m";
    static const std::string TE_COL_BRIGHT_WHITE =   "\x1B[97m";

    // format terminator
    static const std::string TE_COL_END = "\x1B[0m";

    // error headers
    static const std::string TE_COL_TRACE_TITLE =   "\x1B[3;44;97m";
    static const std::string TE_COL_LOG_TITLE =     "\x1B[3;42;97m";
    static const std::string TE_COL_MESSAGE_TITLE = "\x1B[3;107;30m";
    static const std::string TE_COL_WARN_TITLE =    "\x1B[3;43;97m";
    static const std::string TE_COL_ERROR_TITLE =   "\x1B[3;41;97m";
    static const std::string TE_COL_FATAL_TITLE =   "\x1B[3;45;97m";

    // text highlighters
    static const std::string TE_COL_HIGHLIGHT_YELLOW = "\x1B[3;43;30m";
    static const std::string TE_COL_HIGHLIGHT_GREEN = "\x1B[3;102;30m";
    static const std::string TE_COL_HIGHLIGHT_CYAN = "\x1B[3;46;30m";
    static const std::string TE_COL_HIGHLIGHT_RED = "\x1B[3;41;97m";

    class GlobalCasters {
    public:
        GlobalCasters() {}
        ~GlobalCasters() {}

        static std::string castConstructorTitle(std::string message) {
            return TE_COL_GREEN + std::string("====================[") + message + std::string("]====================\n") + TE_COL_END;
        }

        static std::string castVkAttach(std::string message) {
            return TE_COL_BRIGHT_CYAN + std::string("====================[") + message + std::string("]====================\n") + TE_COL_END;
        }

        static std::string castVkBuild(std::string message) {
            return TE_COL_BRIGHT_YELLOW + std::string("====================[") + message + std::string("]====================\n") + TE_COL_END;
        }

        static std::string castVkDetach(std::string message) {
            return TE_COL_BRIGHT_MAGENTA + std::string("====================[") + message + std::string("]====================\n") + TE_COL_END;
        }

        static std::string castDestructorTitle(std::string message) {
            return TE_COL_BRIGHT_BLUE + std::string("====================[") + message + std::string("]====================\n") + TE_COL_END;
        }

        static std::string castValicationLayer(std::string message) {
            return castHighlightYellow("[Validation layer]") + message;
        }

        static std::string castHighlightYellow(std::string message) { return TE_COL_HIGHLIGHT_YELLOW + message + TE_COL_END; }
        static std::string castHighlightGreen(std::string message) { return TE_COL_HIGHLIGHT_GREEN + message + TE_COL_END; }
        static std::string castHighlightCyan(std::string message) { return TE_COL_HIGHLIGHT_CYAN + message + TE_COL_END; }
        static std::string castHighlightRed(std::string message) { return TE_COL_HIGHLIGHT_RED + message + TE_COL_END; }
        static std::string castTraceTitle(std::string message) { return TE_COL_TRACE_TITLE + message + TE_COL_END; }
        static std::string castLogTitle(std::string message) { return TE_COL_LOG_TITLE + message + TE_COL_END; }
        static std::string castMessageTitle(std::string message) { return TE_COL_MESSAGE_TITLE + message + TE_COL_END; }
        static std::string castWarnTitle(std::string message) { return TE_COL_WARN_TITLE + message + TE_COL_END; }
        static std::string castErrorTitle(std::string message) { return TE_COL_ERROR_TITLE + message + TE_COL_END; }
        static std::string castFatalTitle(std::string message) { return TE_COL_FATAL_TITLE + message + TE_COL_END; }
        static std::string castBlack(std::string message) { return TE_COL_BLACK + message + TE_COL_END; }
        static std::string castRed(std::string message) { return TE_COL_RED + message + TE_COL_END; }
        static std::string castGreen(std::string message) { return TE_COL_GREEN + message + TE_COL_END; }
        static std::string castYellow(std::string message) { return TE_COL_YELLOW + message + TE_COL_END; }
        static std::string castBlue(std::string message) { return TE_COL_BLUE + message + TE_COL_END; }
        static std::string castMagenta(std::string message) { return TE_COL_MAGENTA + message + TE_COL_END; }
        static std::string castCyan(std::string message) { return TE_COL_CYAN + message + TE_COL_END; }
        static std::string castWhite(std::string message) { return TE_COL_WHITE + message + TE_COL_END; }
    };
}