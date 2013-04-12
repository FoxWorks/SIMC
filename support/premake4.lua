-- Create standalone solution
if SIMC_STANDALONE ~= false then
  solution "simc"
     dofile("premake4_common.lua")
end


--------------------------------------------------------------------------------
-- Simulation Core
--------------------------------------------------------------------------------
project "simc"
   kind "StaticLib"
   language "C++"
   includedirs { "../include" }
   files { "../source/sim_core/**",
           "../include" }
   defines { "SIMC_LIBRARY" }
 
   configuration "windows"
      includedirs { "../external/tinyxml" }
      files { "../source/**",
              "../external/tinyxml/tiny*.cpp" }
   configuration "not windows"
      links { "tinyxml" }
