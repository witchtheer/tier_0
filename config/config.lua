build_dir = "%{wks.location}/build" 

filter {"toolset:gcc"}
  compiler_name = "gcc"
filter {"toolset:clang"}
  compiler_name = "clang"
filter {"toolset:msc"}
  compiler_name = "msc"
filter {}

output_paths = 
{
  OBJDIR = build_dir .. "/Intermediate/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}",
  TARGET = build_dir .. "/Binaries/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" 
}

unix_compiler_settings = {
  debug   =   { "-g", "-Og", "-DDEBUG", "-Wall", "-Wextra", "-pipe", "-fno-omit-frame-pointer", "-fno-inline", "-fdiagnostics-color=always" },
  strict =    { "-g",   "-O0",  "-DDEBUG",    "-Wall",  "-Wextra", "-Werror","-Wpedantic", "-pipe", "-fdiagnostics-color=always"},
  release =   { "-O2",  "-DNDEBUG", "-Wall",  "-Wextra","-pipe",  "-fdiagnostics-color=always"},
  shipping =  { "-O3",  "-flto", "-DSHIPPING","-pipe",  "-fdiagnostics-color=always"} 
}

unix_linker_settings = {
  debug =   {"-g"},
  strict =  {"-g"},
  release = {},
  shipping = {"-flto=full", "-s"}
}

msvc_settings = {
  debug = { "/Od", "/Zi", "/DDEBUG", "/W3", "/MDd" },
  strict = { "/Od", "/Zi", "/DDEBUG", "/W4", "/MDd" },
  release = { "/O2", "/DNDEBUG", "/W3", "/MD" },
  shipping = { "/Ox", "/GL", "/DNDEBUG", "/DSHIPPING", "/W1", "/MD" }
}

msvc_linker_settings = {
  debug = { "/DEBUG" },
  strict = { "/DEBUG" }, 
  release = {},
  shipping = { "/LTCG", "/OPT:REF", "/OPT:ICF" }
}

filter "platforms:x86"
  defines {"ARCH_X86"}
  architecture "x86"
  
filter "platforms:x86_64"
  defines {"ARCH_X86_64"}
  architecture "x86_64"
filter {}

targetdir(output_paths.TARGET)
objdir(output_paths.OBJDIR)

-- GCC / Clang
filter { "toolset:gcc or clang", "configurations:Debug" }
  buildoptions(unix_compiler_settings.debug)
  linkoptions(unix_linker_settings.debug)

filter { "toolset:gcc or clang", "configurations:Strict" }
  buildoptions(unix_compiler_settings.strict)
  linkoptions(unix_linker_settings.strict)
  
filter { "toolset:gcc or clang", "configurations:Release" }
  buildoptions(unix_compiler_settings.release)
  linkoptions(unix_linker_settings.release)
  
filter { "toolset:gcc or clang", "configurations:Shipping" }
  buildoptions(unix_compiler_settings.shipping)
  linkoptions(unix_linker_settings.shipping)

filter { "toolset:msc", "configurations:Debug" }
  buildoptions(msvc_settings.debug)
  linkoptions(msvc_linker_settings.debug)

filter { "toolset:msc", "configurations:Strict" }
  buildoptions(msvc_settings.strict)
  linkoptions(msvc_linker_settings.strict)
  
filter { "toolset:msc", "configurations:Release" }
  buildoptions(msvc_settings.release)
  linkoptions(msvc_linker_settings.release)
  
filter { "toolset:msc", "configurations:Shipping" }
  buildoptions(msvc_settings.shipping)
  linkoptions(msvc_linker_settings.shipping)

filter {}

cdialect "c11"
cppdialect "c++11"

newaction {
    trigger = "clang-check-all",
    description = "Check all C/C++ files in directory tree",
    
    onWorkspace = function(wks)
        if os.target() ~= "linux" then
            print("clang-check-all action is only available on Linux")
            return
        end
        
        local result = os.outputof("which clang-check 2>/dev/null")
        if not result or result == "" then
            print("Error: clang-check not found. Please install clang-tools")
            return
        end
        
        print("Finding and checking all C/C++ files...")
        
        local find_cmd = "find . -name '*.cpp' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' | grep -v build"
        local handle = io.popen(find_cmd)
        local files_output = handle:read("*a")
        handle:close()
        
        if files_output == "" then
            print("No C/C++ files found!")
            return
        end
        
        local issues_found = 0
        
        for file in files_output:gmatch("[^\r\n]+") do
            if os.isfile(file) then
                print(string.format("Checking: %s", file))
                
                local ext = string.match(file, "%.([^%.]+)$"):lower()
                local cmd
                
                local base_flags = "-I./dev/Engine -I./dev/Engine/Core -I./dev/Games"
                
                if ext == "cpp" or ext == "hpp" then
                    if ext == "hpp" then
                        cmd = string.format("clang-check -analyze %s -- -x c++ -std=c++17 -fsyntax-only %s", file, base_flags)
                    else
                        cmd = string.format("clang-check -analyze %s -- -x c++ -std=c++17 %s", file, base_flags)
                    end
                elseif ext == "c" or ext == "h" then
                    local is_cpp_header = false
                    if ext == "h" then
                        local f = io.open(file, "r")
                        if f then
                            local content = f:read("*a")
                            f:close()
                            if content:match("namespace") or content:match("class") or content:match("template") then
                                is_cpp_header = true
                            end
                        end
                    end
                    
                    if ext == "h" and is_cpp_header then
                        cmd = string.format("clang-check -analyze %s -- -x c++ -std=c++17 -fsyntax-only %s", file, base_flags)
                    elseif ext == "h" then
                        cmd = string.format("clang-check -analyze %s -- -x c -fsyntax-only %s", file, base_flags)
                    else
                        cmd = string.format("clang-check -analyze %s -- -x c %s", file, base_flags)
                    end
                end
                
                local check_handle = io.popen(cmd .. " 2>&1")
                local output = check_handle:read("*a")
                local success, exit_type, exit_code = check_handle:close()
                
                if exit_code == 0 then
                    print("  ✓ No issues")
                else
                    print("  ⚠ Issues found:")
                    if output and output ~= "" then
                        for line in output:gmatch("[^\r\n]+") do
                            if line:match("warning:") or line:match("error:") or line:match("note:") then
                                print("    " .. line)
                            end
                        end
                    end
                    issues_found = issues_found + 1
                end
            end
        end
        
        if issues_found > 0 then
            print(string.format("\nSummary: Issues found in %d file(s)", issues_found))
        else
            print("\nSummary: All files clean! ✓")
        end
    end
}

newaction {
    trigger = "cloc-native",
    description = "Count lines of native code (C/C++/Assembly only)",
    
    onWorkspace = function(wks)
        local result = os.outputof("which cloc 2>/dev/null")
        if not result or result == "" then
            print("Error: cloc not found. Please install cloc")
            print("  Ubuntu/Debian: sudo apt install cloc")
            print("  macOS: brew install cloc")
            print("  Arch: sudo pacman -S cloc")
            return
        end
        
        print("Counting native code lines...")
        
        -- Run cloc with native languages only
        local cmd = 'cloc --include-lang="C++,C/C++ Header,C,Assembly" .'
        local handle = io.popen(cmd)
        local output = handle:read("*a")
        local success, exit_type, exit_code = handle:close()
        
        if exit_code == 0 then
            print(output)
        else
            print("Error running cloc command")
        end
    end
}

newaction {
    trigger = "cloc-all", 
    description = "Count all lines of code in project",
    
    onWorkspace = function(wks)
        local result = os.outputof("which cloc 2>/dev/null")
        if not result or result == "" then
            print("Error: cloc not found. Please install cloc")
            return
        end
        
        print("Counting all code lines...")
        
        local cmd = 'cloc .'
        local handle = io.popen(cmd)
        local output = handle:read("*a")
        local success, exit_type, exit_code = handle:close()
        
        if exit_code == 0 then
            print(output)
        else
            print("Error running cloc command")
        end
    end
}

newaction {
    trigger = "clang-tidy-all",
    description = "Run clang-tidy on all C/C++ files in directory tree",
    
    onWorkspace = function(wks)
        if os.target() ~= "linux" then
            print("clang-tidy-all action is only available on Linux")
            return
        end
        
        local result = os.outputof("which clang-tidy 2>/dev/null")
        if not result or result == "" then
            print("Error: clang-tidy not found. Please install clang-tidy")
            print("  Ubuntu/Debian: sudo apt install clang-tidy")
            print("  Arch: sudo pacman -S clang")
            return
        end
        
        print("Finding and analyzing all C/C++ files with clang-tidy...")
        
        local find_cmd = "find . -name '*.cpp' -o -name '*.c' -o -name '*.h' -o -name '*.hpp' | grep -v build"
        local handle = io.popen(find_cmd)
        local files_output = handle:read("*a")
        handle:close()
        
        if files_output == "" then
            print("No C/C++ files found!")
            return
        end
        
        local issues_found = 0
        local total_files = 0
        
        -- Use .clang-tidy config file if it exists
        local config_file = ".clang-tidy"
        local checks = ""
        if os.isfile(config_file) then
            print(string.format("Using configuration from %s", config_file))
        else
            print("No .clang-tidy file found, using default checks")
            checks = "-checks=-*,readability-*,performance-*,modernize-*,bugprone-*,clang-analyzer-*,cppcoreguidelines-*"
        end
        
        for file in files_output:gmatch("[^\r\n]+") do
            if os.isfile(file) then
                total_files = total_files + 1
                print(string.format("Analyzing: %s", file))
                
                local ext = string.match(file, "%.([^%.]+)$"):lower()
                local cmd
                
                local base_flags = "-I./include -I./src"
                
                if ext == "cpp" or ext == "hpp" then
                    if ext == "hpp" then
                        cmd = string.format("clang-tidy %s %s -- -x c++ -std=c++17 -fsyntax-only %s", checks, file, base_flags)
                    else
                        cmd = string.format("clang-tidy %s %s -- -x c++ -std=c++17 %s", checks, file, base_flags)
                    end
                elseif ext == "c" or ext == "h" then
                    local is_cpp_header = false
                    if ext == "h" then
                        local f = io.open(file, "r")
                        if f then
                            local content = f:read("*a")
                            f:close()
                            if content:match("namespace") or content:match("class") or content:match("template") then
                                is_cpp_header = true
                            end
                        end
                    end
                    
                    if ext == "h" and is_cpp_header then
                        cmd = string.format("clang-tidy %s %s -- -x c++ -std=c++17 -fsyntax-only %s", checks, file, base_flags)
                    elseif ext == "h" then
                        cmd = string.format("clang-tidy %s %s -- -x c -fsyntax-only %s", checks, file, base_flags)
                    else
                        cmd = string.format("clang-tidy %s %s -- -x c %s", checks, file, base_flags)
                    end
                end
                
                local tidy_handle = io.popen(cmd .. " 2>&1")
                local output = tidy_handle:read("*a")
                local success, exit_type, exit_code = tidy_handle:close()
                
                -- clang-tidy returns 0 even with warnings, so we check output content
                local has_warnings = output and (output:match("warning:") or output:match("error:"))
                
                if not has_warnings or output == "" then
                    print("  ✓ No issues")
                else
                    print("  ⚠ Issues found:")
                    if output and output ~= "" then
                        for line in output:gmatch("[^\r\n]+") do
                            if line:match("warning:") or line:match("error:") or line:match("note:") then
                                print("    " .. line)
                            end
                        end
                    end
                    issues_found = issues_found + 1
                end
            end
        end
        
        print(string.format("\nAnalyzed %d file(s)", total_files))
        if issues_found > 0 then
            print(string.format("Summary: Issues found in %d file(s)", issues_found))
        else
            print("Summary: All files clean! ✓")
        end
    end
}

newaction {
    trigger = "help",
    description = "Show available custom actions and usage",
    
    execute = function()
        print("=== Custom Premake5 Actions ===")
        print("")
        print("Code Analysis:")
        print("  premake5 clang-check-all   - Check all C/C++ files with clang-check")
        print("  premake5 clang-tidy-all    - Run clang-tidy on all C/C++ files")
        print("")
        print("Code Statistics:")
        print("  premake5 cloc-native       - Count lines of native code (C/C++/Assembly)")
        print("  premake5 cloc-all          - Count all lines of code in project")
        print("")
        print("Build Systems:")
        print("  premake5 gmake2            - Generate GNU Makefiles")
        print("  premake5 ninja             - Generate Ninja build files")
        print("  premake5 vs2019            - Generate Visual Studio 2019 solution")
        print("  premake5 vs2022            - Generate Visual Studio 2022 solution")
        print("")
        print("Other:")
        print("  premake5 help              - Show this help message")
        print("")
        print("Examples:")
        print("  premake5 ninja && ninja")
        print("  premake5 gmake2 && make config=release")
        print("  premake5 clang-tidy-all")
        print("")
    end
}

function increment_project_counter()
  _G.project_count = _G.project_count + 1
end
