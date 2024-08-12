import os
import asyncio
import platform
import struct

PRINT_COMPILE_OUPTUT = True
PRINT_COMPILE_ERR = False

class PlatformLinux:
    def __init__(self, module_name:str):
        self.prog = "glslangValidator"
        self.module_name = module_name
        self.script_path = os.path.split(os.path.realpath(__file__))[0]

class PlatformWindows:
    def __init__(self, module_name:str):
        # current_binary_dir = current_binary_dir.replace("/", "\\")
        self.prog = "glslangValidator"
        self.module_name = module_name
        self.script_path = os.path.split(os.path.realpath(__file__))[0]

class ToSPV:
    def __init__(self, module_name):

        if platform.system() == "Linux":
            sys_choice = PlatformLinux(module_name)
        elif platform.system() == "Windows":
            sys_choice = PlatformWindows(module_name)
        else:
            print("Unsupported platform!")
            exit(-1)

        self.script_path = sys_choice.script_path
        self.prog = sys_choice.prog

    async def run(self, cmd):
        proc = await asyncio.create_subprocess_shell(\
            cmd,\
            stdout=asyncio.subprocess.PIPE,\
            stderr=asyncio.subprocess.PIPE)

        stdout, stderr = await proc.communicate()
        # print(f'[{cmd!r} exited with {proc.returncode}]')
        if stdout and PRINT_COMPILE_OUPTUT:
            outp = stdout.decode()
            if(outp.find("ERROR") >= 0):
                print(f'[stdout] {outp}')
        if stderr and PRINT_COMPILE_ERR:
            print(f'[stderr] {stderr.decode()}')

    def compile(self):
        head = """
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace VK4 {
	class ShaderBin {
	public:
		static inline std::unordered_map<std::string, std::vector<unsigned char>> S {
"""
        lines = []
        tail = """
		};
	};
}
"""
        template = "\"{0}\", {1}"

        for parent in os.walk(self.script_path):
            subfolders = list(filter(lambda f: f.find("_") < 0, parent[1]))

            for folder in subfolders:
                p = self.script_path + "/" + folder + "/"
                for obj in os.walk(p):
                    frag_files = list(filter(lambda f: f.find(".frag") >= 0, obj[2]))
                    vert_files = list(filter(lambda f: f.find(".vert") >= 0, obj[2]))
                    if frag_files:
                        for frag_file in frag_files:
                            asyncio.run(self.run(' '.join([self.prog, "-V", p + frag_file])))
                            fn = frag_file.split('.')[0] + ".frag.spv"
                            with open("frag.spv", 'rb') as ff:
                                xx = ff.read()
                                data = "{" + str(struct.unpack("=" + str(len(xx)) + "B", xx))[1:-1] + "}"
                                lines.append("\t\t\t{" + template.format(fn, data) + "}")
                            os.remove("frag.spv")
                    if vert_files:
                        for vert_file in vert_files:
                            asyncio.run(self.run(' '.join([self.prog, "-V", p + vert_file])))
                            fn = vert_file.split('.')[0] + ".vert.spv"
                            with open("vert.spv", 'rb') as ff:
                                xx = ff.read()
                                data = "{" + str(struct.unpack("=" + str(len(xx)) + "B", xx))[1:-1] + "}"
                                lines.append("\t\t\t{" + template.format(fn, data) + "}")
                            os.remove("vert.spv")

            ll = ",\n".join(lines)
            new_file = head + ll + tail
            if os.path.exists(self.script_path + "/Vk_ShaderBin"):
                with open(self.script_path + "/Vk_ShaderBin", 'r') as rr:
                    old_file = rr.read()
            else:
                old_file = ""
            if old_file != new_file:
                with open(self.script_path + "/Vk_ShaderBin", 'w') as ff:
                    ff.write(new_file)

if __name__ == "__main__":
    ToSPV("pyvk").compile()
