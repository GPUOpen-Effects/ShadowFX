_AMD_LIBRARY_NAME = "ShadowFX"
_AMD_LIBRARY_NAME_ALL_CAPS = string.upper(_AMD_LIBRARY_NAME)
_AMD_D3D_VERSION = "d3d12"

-- Set _AMD_LIBRARY_NAME and _AMD_D3D_VERSION before including amd_premake_util.lua
dofile ("../../premake/amd_premake_util.lua")

workspace ("AMD_" .. _AMD_LIBRARY_NAME .. "12")
   configurations { "DLL_Debug", "DLL_Release", "Lib_Debug", "Lib_Release", "DLL_Release_MT" }
   platforms { "Win32", "x64" }
   location "../build"
   filename ("AMD_" .. _AMD_LIBRARY_NAME .. "12" .. _AMD_VS_SUFFIX)
   startproject ("AMD_" .. _AMD_LIBRARY_NAME .. "12")

   filter "platforms:Win32"
      system "Windows"
      architecture "x86"

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

project ("AMD_" .. _AMD_LIBRARY_NAME .. "12")
   language "C++"
   location "../build"
   filename ("AMD_" .. _AMD_LIBRARY_NAME .. "12" .. _AMD_VS_SUFFIX)
   uuid "21473363-E6A1-4460-8454-0F4C411B5B3D"
   targetdir "../lib/%{_AMD_LIBRARY_DIR_LAYOUT}"
   objdir "../build/%{_AMD_LIBRARY_DIR_LAYOUT}"
   warnings "Extra"
   exceptionhandling "Off"
   rtti "Off"

   -- Specify WindowsTargetPlatformVersion here for VS2015
   systemversion (_AMD_WIN_SDK_VERSION_FOR_D3D12)

   files { "../inc/**.h", "../src/AMD_%{_AMD_LIBRARY_NAME}_Precompiled.h", "../src/AMD_%{_AMD_LIBRARY_NAME}12*.h", "../src/AMD_%{_AMD_LIBRARY_NAME}12*.cpp", "../src/Shaders/**.hlsl" }
   includedirs { "../inc", "../../amd_lib/shared/common/inc", "../../amd_lib/shared/%{_AMD_D3D_VERSION}/inc" }
   defines { "AMD_SHADOWFX_D3D12" }

   filter "configurations:DLL_*"
      kind "SharedLib"
      defines { "_USRDLL" }
      links { "d3d12"}
      -- Copy DLL and import library to the lib directory
      postbuildcommands { amdLibPostbuildCommands() }
      postbuildmessage "Copying build output to lib directory..."

   filter "configurations:Lib_*"
      kind "StaticLib"
      defines { "_LIB", "AMD_SHADOWFX_COMPILE_STATIC_LIB" }

   filter "configurations:*_Debug"
      defines { "WIN32", "_DEBUG", "_WINDOWS", "_WIN32_WINNT=0x0A00" }
      flags { "FatalWarnings" }
      symbols "On"
      characterset "Unicode"
      -- add "d" to the end of the library name for debug builds
      targetsuffix "d"

   filter "configurations:*_Release"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_WIN32_WINNT=0x0A00" }
      flags { "FatalWarnings" }
      characterset "Unicode"
      optimize "On"

   filter "configurations:DLL_Release_MT"
      defines { "WIN32", "NDEBUG", "_WINDOWS", "_WIN32_WINNT=0x0A00" }
      flags { "FatalWarnings" }
      characterset "Unicode"
      -- link against the static runtime to avoid introducing a dependency
      -- on the particular version of Visual Studio used to build the DLLs
      flags { "StaticRuntime" }
      optimize "On"

   filter "action:vs*"
      -- specify exception handling model for Visual Studio to avoid
      -- "'noexcept' used with no exception handling mode specified" 
      -- warning in vs2015
      buildoptions { "/EHsc" }

   filter "platforms:Win32"
      targetname "%{_AMD_LIBRARY_PREFIX}%{_AMD_LIBRARY_NAME}12_x86"

   filter "platforms:x64"
      targetname "%{_AMD_LIBRARY_PREFIX}%{_AMD_LIBRARY_NAME}12_x64"
