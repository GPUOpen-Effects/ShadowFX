dofile ("../../../../premake/amd_premake_util.lua")

workspace "dx12u"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "../build"
   filename ("dx12u" .. _AMD_VS_SUFFIX)
   startproject "dx12u"

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

project "dx12u"
   kind "StaticLib"
   language "C++"
   location "../build"
   filename ("dx12u" .. _AMD_VS_SUFFIX)
   uuid "5937530F-C54B-7BC0-8ECD-1E20FAA077F1"
   targetdir "../lib"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"
   disablewarnings { "4201", "4996" }
   defines { "NOMINMAX" }

   -- Specify WindowsTargetPlatformVersion here for VS2015
   windowstarget (_AMD_WIN_SDK_VERSION_FOR_D3D12)

   files { "../inc/**.hpp", "../inc/**.ipp", "../src/**.hpp", "../src/**.cpp", "../src/**.hlsl" }
   includedirs { "../inc", "../../gu/inc", "../../tml/inc", "../../../../amd_lib/shared/d3d12/inc" }

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0A00" }
      flags { "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_LIB", "_WIN32_WINNT=0x0A00" }
      flags { "LinkTimeOptimization", "Symbols", "FatalWarnings", "Unicode" }
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"
