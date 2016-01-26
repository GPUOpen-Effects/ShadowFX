fxc /T ps_5_0 /E AMD_SHADOWFX_FILTERING_POINT /O1 /Fh ..\inc\AMD_SHADOWS_CUBE_FILTERING_POINT.inc /Vn AMD_SHADOWS_CUBE_FILTERING_POINT_Data ..\AMD_ShadowsCube.hlsl

fxc /E AMD_SHADOWFX_FILTERING_POINT /O1 /P ShadowFilteringUsingTextureArray.hlsl ..\AMD_ShadowsCube.hlsl
 