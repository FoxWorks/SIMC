-- Create standalone solution
if SIMC_STANDALONE ~= false then
	solution "simc"
		dofile("premake5_common.lua")
end


--------------------------------------------------------------------------------
-- Simulation Core
--------------------------------------------------------------------------------
project "simc"
	uuid "00058543-E5EA-1540-B535-BCE859AA319E"
	kind "StaticLib"
	language "C++"
	includedirs {
		"../include",
		"../external/tinyxml"
	}
	files {
		"../source/**",
		"../include/**",
		"../external/tinyxml/tiny*.cpp",
	}
	defines { "SIMC_LIBRARY" }