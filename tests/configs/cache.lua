group "tests"

project "cache_1"
  kind "ConsoleApp"
  language "C"

  files {"../cache/data_1.c"}
  includedirs {"%{wks.location}/include"}
  links {"tier_0"}

  increment_project_counter()
