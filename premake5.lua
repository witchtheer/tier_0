require "premake-ninja/ninja"

include "config/config.lua"
_G.project_count = 0

workspace "tier_0"
  configurations {"Debug" , "Release" , "Shipping", "Strict"}
  platforms {"x86", "x86_64"}

  toolset "clang"

  defaultplatform "x86_64"
  
  include "src/tier_0.lua"
  include "tests/tests.lua"

  print("project count: ", _G.project_count)
