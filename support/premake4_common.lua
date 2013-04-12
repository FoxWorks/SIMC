-- Get file version from the header
function get_header_version(filename,tag)
  local header = io.open(filename,"r")
  local contents = header:read("*all")
  header:close()

  -- Get version
  local pos = string.find(contents,tag)
  contents = string.sub(contents,pos+#tag)
  pos = string.find(contents,"\"")
  contents = string.sub(contents,pos+1)
  return string.sub(contents,1,string.find(contents,"\"")-1)
end


-- Check if solution name is defined
SOLUTION_NAME = SOLUTION_NAME or "solution"


-- Create default solution
solution (SOLUTION_NAME)
   platforms { "x32", "x64" }
   configurations {
     "Debug",
     "Release",
     "DebugDynamic",
     "ReleaseDynamic",

     "DebugSingleThread",
     "ReleaseSingleThread",
     "DebugSingleThreadDynamic",
     "ReleaseSingleThreadDynamic"
   }

   -- Setup environment
   location (_ACTION)
   targetdir "../bin"
   debugdir ("../"..SOLUTION_NAME)
   vpaths { ["**"]  = "../source/**" }

   -- Default include path
   includedirs { "../source" }

   -- Debug/Release configurations and correct debug suffix
   configuration "Debug*"
      defines { "DEBUG" }
      flags { "Symbols" }
   configuration "Release*"
      flags { "Optimize", "Symbols" }

   -- Generate suffixes
   configuration { "x64", "Debug*" }
      targetsuffix "d"
   configuration { "x32", "Debug*" }
      targetsuffix "d32"
   configuration { "x32", "Release*" }
      targetsuffix "32"
   configuration { "x64", "*SingleThread*", "Debug*" }
      targetsuffix "_std"
   configuration { "x64", "*SingleThread*", "Release*" }
      targetsuffix "_st"
   configuration { "x32", "*SingleThread*", "Debug*" }
      targetsuffix "_std32"
   configuration { "x32", "*SingleThread*", "Release*" }
      targetsuffix "_st32"

   -- Windows-specific
   configuration { "windows" }
      defines { "_CRT_SECURE_NO_WARNINGS","WIN32" }
   configuration { "windows", "x64" }
      defines { "WIN64" }

   -- Linux specific
   configuration { "not windows" }
      links { "m", "pthread" }
      linkoptions { "-lstdc++" }

   -- Default configuration for libraries
   function library()
     configuration { "not *Dynamic*" }
        kind "StaticLib"
     configuration { "*Dynamic*" }
        kind "SharedLib"
     configuration {}
   end
