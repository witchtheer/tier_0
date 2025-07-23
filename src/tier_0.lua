group "lib"

project "tier_0"
  kind "StaticLib"
  language "C++"

  print("Current working directory:", os.getcwd())
  print("Looking for files in src/perf/...")
  
  local perf_files = os.matchfiles("perf/**.c")
  print("Found perf C files:", table.concat(perf_files, ", "))
  
  filter "system:linux"
    files
    {
      "platform/linux/*.cpp",
      "platform/linux/*.c",
      "platform/linux/*.h",
      "platform/linux/*.hpp"
    }

  filter "system:windows"
    files
    {
      "platform/windows/*.cpp",
      "platform/windows/*.c",
      "platform/windows/*.h",
      "platform/windows/*.hpp"
    }

  filter "system:macosx"
    files 
    {
      "platform/mac/*.cpp",
      "platform/mac/*.c",
      "platform/mac/*.h",
      "platform/mac/*.hpp"
    }

  -- x86 architecture files (including assembly)
  filter "platforms:x86"
    files
    {
      "perf/arch/x86/*.s",
      "perf/arch/x86/*.c",
      "perf/arch/x86/*.h",
      "perf/arch/x86/*.cpp",
      "perf/arch/x86/*.hpp"
    }

  filter "platforms:x86_64"
    files
    {
      "perf/arch/x86_64/*.s",
      "perf/arch/x86_64/*.c",
      "perf/arch/x86_64/*.h",
      "perf/arch/x86_64/*.cpp",
      "perf/arch/x86_64/*.hpp"
    }

  filter {}
  -- Shared perf code
  files
  {
    "perf/**.cpp",
    "perf/**.c",
    "perf/**.h",
    "perf/**.hpp",
  }

  -- utils
  files 
  {
    "utils/**.cpp",
    "utils/**.c",
    "utils/**.h",
    "utils/**.hpp"
  }

  files 
  {
    "platforms/shared/*.c",
    "platforms/shared/*.h"
  }

  includedirs
  {
    ".",
    "%{wks.location}/include"
  }

  -- Assembly build rules with hardcoded paths
  filter { "files:perf/arch/x86/_counters.s", "platforms:x86" }
    if _ACTION ~= "ninja" then
      buildmessage "Assembling _counters.s"
    end
    buildcommands
    {
      "mkdir -p %{cfg.objdir} && clang -c -m32 src/perf/arch/x86/_counters.s -o %{cfg.objdir}/_counters.o"
    }
    buildoutputs
    {
      "%{cfg.objdir}/_counters.o"
    }
    buildinputs
    {
      "perf/arch/x86/_counters.s"
    }

  filter { "files:perf/arch/x86_64/_counters.s", "platforms:x86_64" }
    if _ACTION ~= "ninja" then
      buildmessage "Assembling _counters.s"
    end
    buildcommands 
    {
      "mkdir -p %{cfg.objdir} && clang -c -m64 src/perf/arch/x86_64/_counters.s -o %{cfg.objdir}/_counters.o"
    }
    buildoutputs
    {
      "%{cfg.objdir}/_counters.o"
    }
    buildinputs
    {
      "perf/arch/x86_64/_counters.s"
    }

  filter {}

increment_project_counter()
