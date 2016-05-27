dofile ("../../../../premake/amd_premake_util.lua")

workspace "gu"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "../build"
   filename ("gu" .. _AMD_VS_SUFFIX)
   startproject "gu"

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

project "gu"
   kind "StaticLib"
   language "C++"
   location "../build"
   filename ("gu" .. _AMD_VS_SUFFIX)
   uuid "01785900-ED03-880B-56B2-880B425D880B"
   targetdir "../lib"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"

   -- Specify WindowsTargetPlatformVersion here for VS2015
   windowstarget (_AMD_WIN_SDK_VERSION_FOR_D3D12)

   files { "../inc/**.hpp", "../src/**.hpp", "../src/**.cpp", "../src/**.hlsl" }
   includedirs { "../src", "../../tml/inc" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0601" }
      flags { "LinkTimeOptimization", "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"
