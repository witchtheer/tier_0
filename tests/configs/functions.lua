group "tests"

project "functions_1"
  kind "ConsoleApp"
  language "C"

  files {"../function/func_start_end.c"}
  includedirs {"%{wks.location}/include"}
  links {"tier_0"}

  increment_project_counter()

project "functions_2"
  kind "ConsoleApp"
  language "C"

  files {"../function/multiple_functions.c"}
  includedirs {"%{wks.location}/include"}
  links {"tier_0"}

  increment_project_counter()

