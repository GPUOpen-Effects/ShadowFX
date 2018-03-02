_AMD_LIBRARY_NAME = "ShadowFX12"
_AMD_LIBRARY_NAME_GENERAL = "ShadowFX"
_AMD_LIBRARY_NAME_GENERAL_ALL_CAPS = string.upper(_AMD_LIBRARY_NAME_GENERAL)

-- Set _AMD_LIBRARY_NAME and _AMD_LIBRARY_NAME_GENERAL before including amd_premake_util.lua
dofile ("../../premake/amd_premake_util.lua")

workspace (_AMD_LIBRARY_NAME .. "_Sample")
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "../build"
   filename (_AMD_LIBRARY_NAME .. "_Sample" .. _AMD_VS_SUFFIX)
   startproject (_AMD_LIBRARY_NAME .. "_Sample")

   filter "platforms:x64"
      system "Windows"
      architecture "x64"

externalproject ("AMD_" .. _AMD_LIBRARY_NAME)
   kind "SharedLib"
   language "C++"
   location "../../AMD_%{_AMD_LIBRARY_NAME_GENERAL}/build"
   filename ("AMD_" .. _AMD_LIBRARY_NAME .. _AMD_VS_SUFFIX)
   uuid "21473363-E6A1-4460-8454-0F4C411B5B3D"
   configmap {
      ["Debug"] = "DLL_Debug",
      ["Release"] = "DLL_Release" }

externalproject "dx12u"
   kind "StaticLib"
   language "C++"
   location "../../framework/d3d12/dx12u/build"
   filename ("dx12u" .. _AMD_VS_SUFFIX)
   uuid "5937530F-C54B-7BC0-8ECD-1E20FAA077F1"

externalproject "gu"
   kind "StaticLib"
   language "C++"
   location "../../framework/d3d12/gu/build"
   filename ("gu" .. _AMD_VS_SUFFIX)
   uuid "01785900-ED03-880B-56B2-880B425D880B"

project (_AMD_LIBRARY_NAME .. "_Sample")
   kind "WindowedApp"
   language "C++"
   location "../build"
   filename (_AMD_LIBRARY_NAME .. "_Sample" .. _AMD_VS_SUFFIX)
   uuid "332B098E-9F8C-E69B-28FA-2C4E945A1D5B"
   targetdir "../bin"
   objdir "../build/%{_AMD_SAMPLE_DIR_LAYOUT}"
   warnings "Extra"
   floatingpoint "Fast"
   disablewarnings { "4238", "4100" } -- allow 4238 in simple samples, simplifies D3D12X

   -- Specify WindowsTargetPlatformVersion here for VS2015
   systemversion (_AMD_WIN_SDK_VERSION_FOR_D3D12)

   -- Copy DLLs to the local bin directory
   postbuildcommands { amdSamplePostbuildCommands(false, true) }
   postbuildmessage "Copying dependencies..."

   -- Use old assimp libraries for 2017
   assimpLibraryName = "assimp_%{cfg.platform}%{_AMD_VS_SUFFIX}"
   if _AMD_VS_SUFFIX == "_2017" then
      assimpLibraryName = "assimp_%{cfg.platform}_2015"
   end

   files { "../src/**.hpp", "../src/**.cpp", "../src/**.hlsl", "../src/**.inl" }
   includedirs { "../../AMD_%{_AMD_LIBRARY_NAME_GENERAL}/inc", "../../amd_lib/shared/common/inc", "../../amd_lib/shared/d3d12/inc", "../../framework/d3d12/dx12u/inc", "../../framework/d3d12/gu/inc", "../../framework/d3d12/tml/inc", "../../third_party/assimp/include" }
   libdirs { "../../third_party/assimp/lib" }
   links { "AMD_%{_AMD_LIBRARY_NAME}", "dx12u", "gu", "d3d12", "d3dcompiler", "dxgi", assimpLibraryName }
   defines { "NOMINMAX", "ASSIMP_DLL", "AMD_SHADOWFX_D3D12", "AMD_%{_AMD_LIBRARY_NAME_GENERAL_ALL_CAPS}_COMPILE_DYNAMIC_LIB=1" }
   entrypoint "mainCRTStartup"

   filter "configurations:Debug"
      defines { "WIN32", "_DEBUG", "DEBUG", "PROFILE", "_WINDOWS", "_WIN32_WINNT=0x0A00" }
      flags { "FatalWarnings" }
      symbols "On"
      characterset "Unicode"
      targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

   filter "configurations:Release"
      defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "_WIN32_WINNT=0x0A00" }
      flags { "LinkTimeOptimization", "FatalWarnings" }
      symbols "On"
      characterset "Unicode"
      targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
      optimize "On"
