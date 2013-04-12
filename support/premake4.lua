-- Create standalone solution
if SIMC_STANDALONE ~= false then
  SOLUTION_NAME = "simc"
  dofile("premake4_common.lua")
end


--------------------------------------------------------------------------------
-- Simulation Core
--------------------------------------------------------------------------------
project "simc"
   kind "StaticLib"
   language "C++"
   files { "../source/sim_core/**" }
   defines { "SIMC_LIBRARY" }
 
   configuration "windows"
      includedirs { "../external/tinyxml" }
      files { "../source/**",
              "../external/tinyxml/tiny*.cpp" }
   configuration "not windows"
      links { "tinyxml" }

   configuration "*Dynamic*"
      defines { "SIMC_DYNAMIC" }
   configuration "*SingleThread*"
      defines { "SIMC_SINGLETHREADED" }
