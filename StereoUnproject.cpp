
#define D3D9
#define WINDOWED

// If app hasn't choosen, set to work with Windows 98, Windows Me, Windows 2000, Windows XP and beyond
#ifndef WINVER
#define WINVER         0x0500
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0600
#endif

#include <stdio.h>

#ifdef D3D9
#include <d3d9.h> 
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#else
#include <d3d10.h>
#include <d3dx10.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#endif

typedef int D3D10_FEATURE_LEVEL1;
typedef enum
{
    NVAPI_DEVICE_FEATURE_LEVEL_NULL       = -1,
    NVAPI_DEVICE_FEATURE_LEVEL_10_0       = 0,
    NVAPI_DEVICE_FEATURE_LEVEL_10_0_PLUS  = 1,
    NVAPI_DEVICE_FEATURE_LEVEL_10_1       = 2,
    NVAPI_DEVICE_FEATURE_LEVEL_11_0       = 3,
} NVAPI_DEVICE_FEATURE_LEVEL;

#include "nvapi.h"
#pragma comment(lib, "nvapi.lib")

#include <tchar.h>


#include "nvStereo.h"

static LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

///////////////////////////////////////////////////////////////////////////////
//  App logic
///////////////////////////////////////////////////////////////////////////////
bool gAnimate = true;
bool gCorrectStereo = true;

///////////////////////////////////////////////////////////////////////////////
//  App Window & d3d managment
///////////////////////////////////////////////////////////////////////////////
HINSTANCE g_hInstance = 0;
HWND g_hWnd=NULL;
StereoHandle g_StereoHandle = 0;
float        g_EyeSeparation = 0;
float        g_Separation = 0;
float        g_Convergence = 0;

#ifdef D3D9

nv::StereoParametersD3D9    g_StereoParamD3D9;

IDirect3D9*            g_D3D9 = NULL;
IDirect3DDevice9*      g_D3D9Device = NULL;
D3DPRESENT_PARAMETERS  g_D3D9PresentParams;

IDirect3DTexture9*     g_D3D9DepthBuffer = NULL;
IDirect3DSurface9*     g_D3D9DepthBufferSurface = NULL;

D3DVIEWPORT9           g_D3D9MainViewport;

ID3DXEffect*           g_pCubeEffect = NULL;
D3DXHANDLE             g_pCubeTechnique = NULL;
D3DXHANDLE             g_pvCubePosParam = NULL;
D3DXHANDLE             g_pmCubeRotParam = NULL;
D3DXHANDLE             g_pmProjMatParam = 0;

static const char g_cubeEffectSrc[] =
    "float4     g_CubePos; \n" \
    "float4x4   g_CubeRot; \n" \
    "float4x4   g_ProjMat; \n" \
    "\n" \
    "struct Fragment{ \n" \
    "    float4 Pos : POSITION; \n" \
    "    float3 Nor : TEXCOORD0; };\n" \
    "\n" \
    "\n" \
    "Fragment VS( float3 Pos : POSITION, float3 Nor : TEXCOORD0)\n" \
    "{\n" \
    "    Fragment f;\n" \
    "    float3 pos = Pos; \n" \
    "    f.Nor = mul( Nor, g_CubeRot).xyz; \n" \
    "    pos = mul( float4(pos*g_CubePos.w, 0), g_CubeRot).xyz + g_CubePos.xyz; \n" \
    "    f.Pos = mul( float4( pos, 1 ), g_ProjMat ); \n"\
    "    return f;\n" \
    "}\n" \
    "\n" \
    "float4 PS( Fragment f ) : COLOR\n" \
    "{\n" \
    "    //float3 dif = abs(g_CubeRot[0].xyz); \n" \
    "    float3 dif = float3(1, 0, 0); \n" \
    "    dif *= ( 0.3 + 0.7 * abs( dot( normalize(f.Nor), normalize(float3(1, 1, -1)) ) )); \n" \
    "    return float4( dif, 1); \n" \
    "}\n" \
    "\n" \
    "technique Render\n" \
    "{\n" \
    "    pass P0\n" \
    "    {\n" \
    "        VertexShader = compile vs_3_0 VS();\n" \
    "        PixelShader  = compile ps_3_0 PS();\n" \
    "    }\n" \
    "}\n" \
    "\n";

ID3DXEffect*           g_pUnprojectEffect = NULL;
D3DXHANDLE             g_pUnprojectTechnique = NULL;
D3DXHANDLE             g_pQuadRect = NULL;
D3DXHANDLE             g_pImageSizeInv = NULL;
D3DXHANDLE             g_pNearFarDepthProj = NULL;
D3DXHANDLE             g_pDepthBuffer = NULL;
D3DXHANDLE             g_pmProjInvMatParam = 0;

D3DXHANDLE             g_pCorrectStereo = NULL;

D3DXHANDLE             g_pStereoEyeSeparation = NULL;
D3DXHANDLE             g_pStereoSeparation = NULL;
D3DXHANDLE             g_pStereoConvergence = NULL;

D3DXHANDLE             g_pStereoParamMap = NULL;

static const char g_unprojectEffectSrc[] =
    "float4     g_QuadRect; \n" \
    "float4     g_ImageSizeInv; \n" \
    "Texture2D  g_DepthMap; \n" \
    "sampler    g_DepthMapSampler = \n" \
    "                sampler_state {\n" \
    "                   Texture = <g_DepthMap>; \n" \
    "                   MipFilter = POINT; \n" \
    "                   MinFilter = POINT; \n" \
    "                   MagFilter = POINT; \n" \
    "                 };\n" \
    "float4x4   g_ProjInv; \n" \
    "float4     g_NearFarDepthProj; \n" \
    "bool       g_CorrectStereo; \n" \
    "float      g_StereoEyeSeparation; \n" \
    "float      g_StereoSeparation; \n" \
    "float      g_StereoConvergence; \n" \
    "Texture2D  g_StereoParamMap; \n" \
    "sampler    g_StereoParamMapSampler = \n" \
    "                sampler_state {\n" \
    "                   Texture = <g_StereoParamMap>; \n" \
    "                   MipFilter = POINT; \n" \
    "                   MinFilter = POINT; \n" \
    "                   MagFilter = POINT; \n" \
    "                 };\n" \
    "\n" \
    "struct Fragment{ \n" \
    "    float4 Pos : SV_POSITION;\n" \
    "    float2 Tex : TEXCOORD0; };\n" \
    "\n" \
    "Fragment VS( float3 Pos : POSITION )\n" \
    "{\n" \
    "    Fragment f;\n" \
    "    f.Tex = Pos; \n"\
    "    f.Pos = float4( g_QuadRect.x + f.Tex.x * g_QuadRect.z, g_QuadRect.y + g_QuadRect.w - f.Tex.y * g_QuadRect.w, 0, 1);\n" \
    "    \n" \
    "    return f;\n" \
    "}\n" \
    "\n" \
    "float4 stereoToMonoNPOS( float4 nposStereo, float Wclip ) : SV_Target\n" \
    "{\n" \
    "    float2 stereoParam = float2( tex2Dlod( g_StereoParamMapSampler, float4( 0.0, 0, 0, 0)).x, tex2Dlod( g_StereoParamMapSampler, float4( 0.125, 0, 0, 0)).x); \n" \
    "    float4 nposMono = nposStereo; \n" \
    "    nposMono.x = nposStereo.x + stereoParam.x + stereoParam.y / Wclip; \n" \
    "    return nposMono; \n" \
    "}\n" \
    "\n" \
    "float4 stereoToMonoCPOS( float4 cposStereo ) : SV_Target\n" \
    "{\n" \
    "    float2 stereoParam = float2( tex2Dlod( g_StereoParamMapSampler, float4( 0.0, 0, 0, 0)).x, tex2Dlod( g_StereoParamMapSampler, float4( 0.125, 0, 0, 0)).x); \n" \
    "//    float stereoSign = tex2Dlod( g_StereoParamMapSampler, float4( 0.25, 0, 0, 0)).x; \n" \
    "//    float2 stereoParam = g_StereoEyeSeparation*g_StereoSeparation*float2(-stereoSign, stereoSign * g_StereoConvergence ); \n" \
    "    float4 cposMono = cposStereo; \n" \
    "    cposMono.x = cposStereo.x + stereoParam.x * cposStereo.w + stereoParam.y; \n" \
    "    return cposMono; \n" \
    "}\n" \
    "\n" \
    "float4 PS( Fragment f ) : COLOR0\n" \
    "{\n" \
    "    // Image space \n" \
    "    float2 ipos = float2( f.Pos.x*g_ImageSizeInv.z, 1 - f.Pos.y*g_ImageSizeInv.w); \n" \
    "    \n" \
    "    // Fetch Znorm and eval Wclip space \n" \
    "    float  Znorm = tex2Dlod( g_DepthMapSampler, float4( ipos.x - 0.5*g_ImageSizeInv.z, (1 - ipos.y) - 0.5*g_ImageSizeInv.w, 0, 0)).x; \n" \
    "    if (Znorm >= 1) discard; \n" \
    "    float  Wclip = g_NearFarDepthProj.w / ( Znorm - g_NearFarDepthProj.z ); \n" \
    "    \n" \
    "    // Normalized space \n" \
    "    float4 nposStereo = float4( ipos*2-1, Znorm, 1 ); \n" \
    "    \n" \
    "    /** \n" \
    "    float4 nposMono = nposStereo; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       nposMono = stereoToMonoNPOS( nposStereo, Wclip ); \n" \
    "    \n" \
    "    // Clip space \n" \
    "    float4 cpos = nposMono*Wclip; \n" \
    "    /**/ \n" \
    "    // Clip space \n" \
    "    float4 cposStereo = nposStereo*Wclip; \n" \
    "    float4 cpos = cposStereo; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       cpos = stereoToMonoCPOS( cposStereo ); \n" \
    "    /**/ \n" \
    "    \n" \
    "    // Eye space \n" \
    "    float4 epos  = mul( cpos, g_ProjInv ); \n" \
    "    \n" \
    "    // Do the bands in eye space \n" \
    "    float bandPos = 2.5 * abs(epos.x % 0.4); \n" \
    "    float alpha = ( abs(2 * bandPos - 1) > 0.8 ? 0.6 : 0 ) ; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       return float4(1, 1, 1, alpha);\n" \
    "    return float4(1, 0.5, 0.5, alpha);\n" \
    "}\n" \
    "\n" \
    "technique Render\n" \
    "{\n" \
    "    pass P0\n" \
    "    {\n" \
    "        AlphaBlendEnable = TRUE; \n" \
    "        SrcBlend = SRCALPHA; \n" \
    "        DestBlend = INVSRCALPHA; \n" \
    "        VertexShader = compile vs_3_0 VS();\n" \
    "        PixelShader  = compile ps_3_0 PS();\n" \
    "    }\n" \
    "}\n" \
    "\n";

#else
nv::StereoParametersD3D10    g_StereoParamD3D10;

ID3D10Device*           g_D3D10Device = NULL;
DXGI_SWAP_CHAIN_DESC    g_DXGISwapChainDesc;
IDXGISwapChain*         g_DXGISwapChain;
ID3D10RenderTargetView* g_D3D10BackBufferRTV;
ID3D10DepthStencilView* g_D3D10DepthBufferDSV;

D3D10_VIEWPORT          g_D3D10MainViewport;

ID3D10ShaderResourceView*           g_D3D10DepthBufferSRV = NULL;

ID3D10Effect*                   g_pCubeEffect = NULL;
ID3D10EffectTechnique*          g_pCubeTechnique = NULL;
ID3D10EffectMatrixVariable*     g_pmCubeRotParam = NULL;
ID3D10EffectMatrixVariable*     g_pmProjMatParam = 0;
ID3D10EffectScalarVariable*     g_psTimeParam = NULL;
ID3D10EffectScalarVariable*     g_psGridCountParam = NULL;
ID3D10EffectScalarVariable*     g_psGridSizeParam = NULL;

static const char g_cubeEffectSrc[] =
   "float4x4   g_CubeRot; \n" \
   "float4x4   g_ProjMat; \n" \
   "float      g_Time; \n" \
   "uint       g_GridCount; \n" \
   "float      g_GridSize; \n" \
   "\n" \
   "const int c_cubeVertices[] = { \n" \
   "    0 + 32, 0 + 32, 2 + 32, 1 + 8, 3 + 8, 5 + 32, 7 + 32, 4 + 8, 6 + 8, 0, 2, \n" \
   "    2, 0,    0 + 16, 1 + 16, 4, 5,    5, 3,    3 + 16, 2 + 16, 7, 6, 6 \n" \
   "   }; \n" \
   "struct Fragment{ \n" \
   "    float4 Pos : SV_POSITION;\n" \
   "    nointerpolation float3 Nor : TEXCOORD0; };\n" \
   "\n" \
   "float4 evaluateCubePos( uint i ) \n" \
   "{ \n" \
   "    float gridInvCount = 1.0 / (g_GridCount-1.0); \n" \
   "    float u = 2.f *gridInvCount *(i % g_GridCount) - 1.f; \n" \
   "    float v = 2.f *gridInvCount *(i / g_GridCount) - 1.f; \n" \
   "    float r = u*u + v*v; \n" \
   "    float instanceOffset =  (1- r) * cos( r *0.5*3.14)*(sin( g_Time *0.05 )); \n" \
   "    float4 cubePos = float4( g_GridSize * ( u ), g_GridSize * ( v ), 5.0f + 3.0f * instanceOffset , g_GridSize * 0.5* gridInvCount ); \n" \
   "    return cubePos; \n" \
   "} \n" \
   "\n" \
   "Fragment VS( uint vertexId : SV_VertexID )\n" \
   "{\n" \
   "    int i = vertexId / 24; \n" \
   "    int v = c_cubeVertices[vertexId % 24]; \n" \
   "    float4 cubePos = evaluateCubePos(i); \n" \
   "    Fragment f;\n" \
   "    float3 pos = float3( -1 + 2 * (v & 0x1), -1 + 2 * (v>>1 & 0x1), -1 + 2 * (v>>2 & 0x1) ); \n" \
   "    float4 nor = float4( pos.x * (v>>3 & 0x1), pos.y * (v>>4 & 0x1), pos.z * (v>>5 & 0x1), 0); \n" \
   "    f.Nor = mul( nor, g_CubeRot).xyz; \n" \
   "    pos = mul( float4(pos*cubePos.w, 0), g_CubeRot).xyz + cubePos.xyz; \n" \
   "    f.Pos = mul( float4( pos, 1 ), g_ProjMat ); \n"\
   "    return f;\n" \
   "}\n" \
   "\n" \
   "float4 PS( Fragment f ) : SV_Target\n" \
   "{\n" \
   "    float3 dif = abs(g_CubeRot[0].xyz) * ( 0.3 + 0.7 * abs( dot( normalize(f.Nor), normalize(float3(1, 1, -1)) ) )); \n" \
   "    return float4( dif, 1); \n" \
   "}\n" \
   "\n" \
   "technique10 Render\n" \
   "{\n" \
   "    pass P0\n" \
   "    {\n" \
   "        SetVertexShader( CompileShader( vs_4_0, VS() ) );\n" \
   "        SetPixelShader( CompileShader( ps_4_0, PS() ) );\n" \
   "    }\n" \
   "}\n" \
   "\n";

ID3D10Effect*                       g_pUnprojectEffect = NULL;
ID3D10EffectTechnique*              g_pUnprojectTechnique = NULL;
ID3D10EffectVectorVariable*         g_pQuadRect = NULL;
ID3D10EffectVectorVariable*         g_pImageSizeInv = NULL;
ID3D10EffectVectorVariable*         g_pNearFarDepthProj = NULL;
ID3D10EffectShaderResourceVariable* g_pDepthBuffer = NULL;
ID3D10EffectMatrixVariable*         g_pmProjInvMatParam = 0;

ID3D10EffectScalarVariable*         g_pCorrectStereo = NULL;

ID3D10EffectScalarVariable*         g_pStereoEyeSeparation = NULL;
ID3D10EffectScalarVariable*         g_pStereoSeparation = NULL;
ID3D10EffectScalarVariable*         g_pStereoConvergence = NULL;

ID3D10EffectShaderResourceVariable* g_pStereoParamMap = NULL;

static const char g_unprojectEffectSrc[] =
    "float4     g_QuadRect; \n" \
    "float4     g_ImageSizeInv; \n" \
    "Texture2D  g_DepthMap; \n" \
    "float4x4   g_ProjInv; \n" \
    "float4     g_NearFarDepthProj; \n" \
    "bool       g_CorrectStereo; \n" \
    "float      g_StereoEyeSeparation; \n" \
    "float      g_StereoSeparation; \n" \
    "float      g_StereoConvergence; \n" \
    "Texture2D  g_StereoParamMap; \n" \
    "\n" \
    "struct Fragment{ \n" \
    "    float4 Pos : SV_POSITION;\n" \
    "    float2 Tex : TEXCOORD0; };\n" \
    "\n" \
    "Fragment VS( uint vertexId : SV_VertexID )\n" \
    "{\n" \
    "    Fragment f;\n" \
    "    f.Tex = float2( 0.f, 0.f); \n"\
    "    if (vertexId == 1) f.Tex.x = 1.f; \n"\
    "    else if (vertexId == 2) f.Tex.y = 1.f; \n"\
    "    else if (vertexId == 3) f.Tex.xy = float2(1.f, 1.f); \n"\
    "    \n" \
    "    f.Pos = float4( g_QuadRect.x + f.Tex.x * g_QuadRect.z, g_QuadRect.y + g_QuadRect.w - f.Tex.y * g_QuadRect.w, 0, 1);\n" \
    "    \n" \
    "    return f;\n" \
    "}\n" \
    "\n" \
    "float4 stereoToMonoNPOS( float4 nposStereo, float Wclip ) : SV_Target\n" \
    "{\n" \
    "    float2 stereoParam = float2( g_StereoParamMap.Load( int3( 0, 0, 0) ).x, g_StereoParamMap.Load( int3( 1, 0, 0) ).x); \n" \
    "    float4 nposMono = nposStereo; \n" \
    "    nposMono.x = nposStereo.x + stereoParam.x + stereoParam.y / Wclip; \n" \
    "    return nposMono; \n" \
    "}\n" \
    "\n" \
    "float4 stereoToMonoCPOS( float4 cposStereo ) : SV_Target\n" \
    "{\n" \
    "    float2 stereoParam = float2( g_StereoParamMap.Load( int3( 0, 0, 0) ).x, g_StereoParamMap.Load( int3( 1, 0, 0) ).x); \n" \
    "    float4 cposMono = cposStereo; \n" \
    "    cposMono.x = cposStereo.x + stereoParam.x * cposStereo.w + stereoParam.y; \n" \
    "    return cposMono; \n" \
    "}\n" \
    "\n" \
    "float4 PS( Fragment f ) : SV_Target\n" \
    "{\n" \
    "    // Image space \n" \
    "    float2 ipos = float2( f.Pos.x*g_ImageSizeInv.z, 1 - f.Pos.y*g_ImageSizeInv.w); \n" \
    "    \n" \
    "    // Fetch Znorm and eval Wclip space \n" \
    "    float  Znorm = g_DepthMap.Load( int3( f.Pos.xy, 0) ).x; \n" \
    "    if (Znorm >= 1) discard; \n" \
    "    float  Wclip = g_NearFarDepthProj.w / ( Znorm - g_NearFarDepthProj.z ); \n" \
    "    \n" \
    "    // Normalized space \n" \
    "    float4 nposStereo = float4( ipos*2-1, Znorm, 1 ); \n" \
    "    \n" \
    "    /** \n" \
    "    float4 nposMono = nposStereo; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       nposMono = stereoToMonoNPOS( nposStereo, Wclip ); \n" \
    "    \n" \
    "    // Clip space \n" \
    "    float4 cpos = nposMono*Wclip; \n" \
    "    /**/ \n" \
    "    // Clip space \n" \
    "    float4 cposStereo = nposStereo*Wclip; \n" \
    "    float4 cpos = cposStereo; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       cpos = stereoToMonoCPOS( cposStereo ); \n" \
    "    /**/ \n" \
    "    \n" \
    "    // Eye space \n" \
    "    float4 epos  = mul( cpos, g_ProjInv ); \n" \
    "    \n" \
    "    // Do the bands in eye space \n" \
    "    float bandPos = 2.5 * abs(epos.x % 0.4); \n" \
    "    float alpha = ( abs(2 * bandPos - 1) > 0.8 ? 0.6 : 0 ) ; \n" \
    "    if ( g_CorrectStereo ) \n" \
    "       return float4(1, 1, 1, alpha);\n" \
    "    return float4(1, 0.5, 0.5, alpha);\n" \
    "}\n" \
    "\n" \
    "BlendState bsBlend \n" \
    "{\n" \
    "   BlendEnable[0] = TRUE;\n" \
    "   SrcBlend = SRC_ALPHA;\n" \
    "   DestBlend = INV_SRC_ALPHA;\n" \
    "};\n" \
    "\n" \
    "technique10 Render\n" \
    "{\n" \
    "    pass P0\n" \
    "    {\n" \
    "        SetBlendState(bsBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );\n" \
    "        SetVertexShader( CompileShader( vs_4_0, VS() ) );\n" \
    "        SetGeometryShader( NULL );\n" \
    "        SetPixelShader( CompileShader( ps_4_0, PS() ) );\n" \
    "    }\n" \
    "}\n" \
    "\n";


#endif

bool CreateWindowAndDevice()
{

    DEVMODE devMode;

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode))
        return false;

    static WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, MsgProc, 0L, 0L,
                             GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                             _T("Test App"), NULL };
    RegisterClassEx( &wc );

    g_hWnd = CreateWindow( wc.lpszClassName, _T("Test App"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Prepare NVAPI for use in this application
    NvAPI_Status status;
    status = NvAPI_Initialize();
    if (status != NVAPI_OK)
    {
        NvAPI_ShortString errorMessage;
        NvAPI_GetErrorMessage(status, errorMessage);
        MessageBoxA(NULL, errorMessage, "Unable to initialize NVAPI", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }
    else
    {
        // Check the Stereo availability
        NvU8 isStereoEnabled;
        status = NvAPI_Stereo_IsEnabled(&isStereoEnabled);

        // Stereo status report an error
        if ( status != NVAPI_OK)
        {
            // GeForce Stereoscopic 3D driver is not installed on the system
            MessageBoxA(NULL, "Stereo is not available\nMake sure the stereo driver is installed correctly", "Stereo not available", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

        }
        // Stereo is available but not enabled, let's enable it
        else if(NVAPI_OK == status && !isStereoEnabled)
        {
            MessageBoxA(NULL, "Stereo is available but not enabled\nLet's enable it", "Stereo not enabled", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            status = NvAPI_Stereo_Enable();
        }

        NvAPI_Stereo_CreateConfigurationProfileRegistryKey(NVAPI_STEREO_DEFAULT_REGISTRY_PROFILE);
    }

#ifdef D3D9

    g_D3D9 = Direct3DCreate9(D3D_SDK_VERSION);

    g_D3D9PresentParams.BackBufferFormat = D3DFMT_A8R8G8B8;
    g_D3D9PresentParams.BackBufferCount = 1;
    g_D3D9PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
    g_D3D9PresentParams.MultiSampleQuality = 0;
    g_D3D9PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_D3D9PresentParams.hDeviceWindow = g_hWnd;
    g_D3D9PresentParams.EnableAutoDepthStencil = FALSE;
    g_D3D9PresentParams.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    g_D3D9PresentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    g_D3D9PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

#ifdef WINDOWED
    RECT rect;
    GetWindowRect( g_hWnd, &rect);

    g_D3D9PresentParams.Windowed = 1;
    g_D3D9PresentParams.FullScreen_RefreshRateInHz = 0;
    g_D3D9PresentParams.BackBufferHeight = rect.bottom - rect.top;
    g_D3D9PresentParams.BackBufferWidth = rect.right - rect.left;
#else
    g_D3D9PresentParams.Windowed = 0;
    g_D3D9PresentParams.FullScreen_RefreshRateInHz = devMode.dmDisplayFrequency;
    g_D3D9PresentParams.BackBufferWidth = devMode.dmPelsWidth;
    g_D3D9PresentParams.BackBufferHeight = devMode.dmPelsHeight;
#endif

    if (g_D3D9 == NULL) {
        MessageBox(NULL,_T("Failed to create D3D9."),_T("Test App Error"),MB_SETFOREGROUND|MB_OK|MB_SYSTEMMODAL|MB_TOPMOST);
        return false;
    }

    if ( FAILED( g_D3D9->CreateDevice(D3DADAPTER_DEFAULT,
                                      D3DDEVTYPE_HAL, 
                                      g_hWnd, 
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING, 
                                      &g_D3D9PresentParams, 
                                      &g_D3D9Device) ) )
    {
        MessageBox(NULL,_T("Failed to create D3D Device."),_T("Test App Error"),MB_SETFOREGROUND|MB_OK|MB_SYSTEMMODAL|MB_TOPMOST);
        exit(0);
        return false;
    }

    status = NvAPI_Stereo_CreateHandleFromIUnknown(g_D3D9Device , &g_StereoHandle);
    if (NVAPI_OK != status )
    {
        MessageBoxA(NULL, "Couldn't create the StereoHandle", "NvAPI_Stereo_CreateHandleFromIUnknown failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }

    status= NvAPI_Stereo_GetEyeSeparation(g_StereoHandle,&g_EyeSeparation );
    if (NVAPI_OK !=  status )
    {
        MessageBoxA(NULL, "Couldn't get the hardware eye separation", "NvAPI_Stereo_GetEyeSeparation failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }

    // Setup the depth buffer
    g_D3D9Device->CreateTexture(g_D3D9PresentParams.BackBufferWidth, g_D3D9PresentParams.BackBufferHeight, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), D3DPOOL_DEFAULT, &g_D3D9DepthBuffer, NULL);

    if ( g_D3D9DepthBuffer )
    {
        g_D3D9DepthBuffer->GetSurfaceLevel(0, &g_D3D9DepthBufferSurface );
    }

    // Setup the viewport
    g_D3D9MainViewport.Width = g_D3D9PresentParams.BackBufferWidth;
    g_D3D9MainViewport.Height = g_D3D9PresentParams.BackBufferHeight;
    g_D3D9MainViewport.MinZ = 0.0f;
    g_D3D9MainViewport.MaxZ = 1.0f;
    g_D3D9MainViewport.X = 0;
    g_D3D9MainViewport.Y = 0;

    // Setup the effect
    {

        DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_DEBUG;
        ID3DXBuffer* pErrors = 0;

        // Cube drawing effect
        D3DXCreateEffect(   g_D3D9Device,
                            (void*) g_cubeEffectSrc, 
                            sizeof(g_cubeEffectSrc), 
                            NULL, 
                            NULL, 
                            dwShaderFlags, 
                            NULL, 
                            &g_pCubeEffect, 
                            &pErrors );

        if( pErrors )
        {
            LPVOID l_pError = NULL;
            l_pError = pErrors->GetBufferPointer(); // then cast to a char* to see it in the locals window
            fprintf(stdout, "Compilation error: \n %s", (char*) l_pError);
        }


        g_pCubeTechnique = g_pCubeEffect->GetTechniqueByName( "Render" );

        g_pvCubePosParam = g_pCubeEffect->GetParameterByName(0, "g_CubePos");
        g_pmCubeRotParam = g_pCubeEffect->GetParameterByName(0, "g_CubeRot");
        g_pmProjMatParam = g_pCubeEffect->GetParameterByName(0, "g_ProjMat");

        // Unproject effect
        D3DXCreateEffect(   g_D3D9Device,
                            (void*) g_unprojectEffectSrc, 
                            sizeof(g_unprojectEffectSrc), 
                            NULL, 
                            NULL, 
                            dwShaderFlags, 
                            NULL, 
                            &g_pUnprojectEffect, 
                            &pErrors );

        if( pErrors )
        {
            LPVOID l_pError = NULL;
            l_pError = pErrors->GetBufferPointer(); // then cast to a char* to see it in the locals window
            fprintf(stdout, "Compilation error: \n %s", (char*) l_pError);
        }


        g_pCubeTechnique = g_pUnprojectEffect->GetTechniqueByName( "Render" );

        g_pImageSizeInv = g_pUnprojectEffect->GetParameterByName(0, "g_ImageSizeInv");
        D3DXVECTOR4 g_ImageSizeInv = D3DXVECTOR4( g_D3D9PresentParams.BackBufferWidth, g_D3D9PresentParams.BackBufferHeight,
                                      1.0 / g_D3D9PresentParams.BackBufferWidth, 1.0 / g_D3D9PresentParams.BackBufferHeight );
        g_pUnprojectEffect->SetVector( g_pImageSizeInv, &g_ImageSizeInv );

        g_pQuadRect = g_pUnprojectEffect->GetParameterByName(0, "g_QuadRect");

        g_pDepthBuffer = g_pUnprojectEffect->GetParameterByName(0, "g_DepthMap");
        g_pUnprojectEffect->SetTexture( g_pDepthBuffer, g_D3D9DepthBuffer );

        g_pmProjInvMatParam = g_pUnprojectEffect->GetParameterByName(0, "g_ProjInv");
        g_pNearFarDepthProj = g_pUnprojectEffect->GetParameterByName(0, "g_NearFarDepthProj");

        g_pCorrectStereo = g_pUnprojectEffect->GetParameterByName(0, "g_CorrectStereo");
        g_pUnprojectEffect->SetBool( g_pCorrectStereo, gCorrectStereo );

        g_pStereoEyeSeparation = g_pUnprojectEffect->GetParameterByName(0, "g_StereoEyeSeparation");
        g_pUnprojectEffect->SetFloat( g_pStereoEyeSeparation, g_EyeSeparation );

        g_pStereoSeparation = g_pUnprojectEffect->GetParameterByName(0, "g_StereoSeparation");
        g_pStereoConvergence = g_pUnprojectEffect->GetParameterByName(0, "g_StereoConvergence");

        g_StereoParamD3D9.createGraphics( g_D3D9Device );
        g_pStereoParamMap = g_pUnprojectEffect->GetParameterByName(0, "g_StereoParamMap");
        g_pUnprojectEffect->SetTexture( g_pStereoParamMap, g_StereoParamD3D9.getStereoParamMapTexture() );

        // Setup projection and Unprojection matrix
        float Near = 2.f;
        float Far = 20.0f;
        float fAspectRatio = (FLOAT)g_D3D9MainViewport.Width / (FLOAT)g_D3D9MainViewport.Height;
        D3DXMATRIX mProj;
        D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI/4, fAspectRatio, Near, Far );

        D3DXMATRIX mProjInv;
        D3DXMatrixInverse( &mProjInv, 0, &mProj );

        g_pCubeEffect->SetMatrix( g_pmProjMatParam, &mProj );
        g_pUnprojectEffect->SetMatrix( g_pmProjInvMatParam, &mProjInv );

        D3DXVECTOR4 vNearFarDepthProj( Near, Far, Far / (Far - Near), -Near*Far / (Far - Near) );
        g_pUnprojectEffect->SetVector( g_pNearFarDepthProj, &vNearFarDepthProj );


        g_D3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_D3D9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
        g_D3D9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    }

#else
    HRESULT hr = S_OK;

    // Select our adapter
    IDXGIAdapter* capableAdapter = NULL;
    {
        // iterate through the candidate adapters
        IDXGIFactory *pFactory;
        hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory) );

        for (UINT adapter = 0; !capableAdapter; ++adapter)
        {
            // get a candidate DXGI adapter
            IDXGIAdapter* pAdapter = NULL;
            hr = pFactory->EnumAdapters(adapter, &pAdapter);
            if (FAILED(hr))
            {
                break;
            }
            // query to see if there exists a corresponding compute device

                    // if so, mark it as the one against which to create our d3d10 device
                capableAdapter = pAdapter;
                capableAdapter->AddRef();

            pAdapter->Release();
        }
        pFactory->Release();
    }

    // Create device and swapchain
    ZeroMemory( &g_DXGISwapChainDesc, sizeof(g_DXGISwapChainDesc) );
    g_DXGISwapChainDesc.BufferCount = 1;
    g_DXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    g_DXGISwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    g_DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    g_DXGISwapChainDesc.OutputWindow = g_hWnd;
    g_DXGISwapChainDesc.SampleDesc.Count = 1;
    g_DXGISwapChainDesc.SampleDesc.Quality = 0;

#ifdef WINDOWED
    RECT rect;
    GetWindowRect( g_hWnd, &rect);

    g_DXGISwapChainDesc.Windowed = true;
    g_DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = devMode.dmDisplayFrequency;
    g_DXGISwapChainDesc.BufferDesc.Width = rect.right - rect.left;
    g_DXGISwapChainDesc.BufferDesc.Height = rect.bottom - rect.top;
#else
    g_DXGISwapChainDesc.Windowed = FALSE;
    //g_DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = devMode.dmDisplayFrequency;
    g_DXGISwapChainDesc.BufferDesc.Width = devMode.dmPelsWidth;
    g_DXGISwapChainDesc.BufferDesc.Height = devMode.dmPelsHeight;
#endif

    hr = D3D10CreateDeviceAndSwapChain(
        capableAdapter,
        D3D10_DRIVER_TYPE_HARDWARE,
        NULL,
        0, //D3D10_CREATE_DEVICE_DEBUG,
        D3D10_SDK_VERSION,
        &g_DXGISwapChainDesc,
        &g_DXGISwapChain,
        &g_D3D10Device);

    capableAdapter->Release();

    status = NvAPI_Stereo_CreateHandleFromIUnknown(g_D3D10Device , &g_StereoHandle);
    if (NVAPI_OK != status )
    {
        MessageBoxA(NULL, "Couldn't create the StereoHandle", "NvAPI_Stereo_CreateHandleFromIUnknown failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }

    if (NVAPI_OK != NvAPI_Stereo_GetEyeSeparation(g_StereoHandle,&g_EyeSeparation ))
    {
        MessageBoxA(NULL, "Couldn't get the hardware eye separation", "NvAPI_Stereo_GetEyeSeparation failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
    }

    // Create the RenderTarget View
    ID3D10Texture2D* lBuffer;
    g_DXGISwapChain->GetBuffer(0, __uuidof( ID3D10Texture2D ), (LPVOID*)&lBuffer );
    if (FAILED(g_D3D10Device->CreateRenderTargetView(lBuffer,
                                                    NULL,
                                                    &g_D3D10BackBufferRTV)))
    {
        MessageBox(NULL,_T("Failed to create RenderTargetView for D3D10."),_T("Test App Error"),MB_SETFOREGROUND|MB_OK|MB_SYSTEMMODAL|MB_TOPMOST);
        return false;
    }
    lBuffer->Release();

    // Create a depthStencil buffer and view
    D3D10_TEXTURE2D_DESC texDesc;
    memset( &texDesc, 0, sizeof(D3D10_TEXTURE2D_DESC) );
    texDesc.Width = g_DXGISwapChainDesc.BufferDesc.Width;
    texDesc.Height = g_DXGISwapChainDesc.BufferDesc.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D10_USAGE_DEFAULT;
    texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;

    ID3D10Texture2D* lSurface = 0;
    g_D3D10Device->CreateTexture2D( &texDesc, 0 , &lSurface);

    D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    memset( &dsvDesc, 0, sizeof(D3D10_DEPTH_STENCIL_VIEW_DESC) );
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    g_D3D10Device->CreateDepthStencilView( lSurface, &dsvDesc, &g_D3D10DepthBufferDSV );

    D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset( &srvDesc, 0, sizeof(D3D10_SHADER_RESOURCE_VIEW_DESC) );
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_D3D10Device->CreateShaderResourceView( lSurface, &srvDesc, &g_D3D10DepthBufferSRV );

    lSurface->Release();

     // Setup the viewport
    g_D3D10MainViewport.Width = g_DXGISwapChainDesc.BufferDesc.Width;
    g_D3D10MainViewport.Height = g_DXGISwapChainDesc.BufferDesc.Height;
    g_D3D10MainViewport.MinDepth = 0.0f;
    g_D3D10MainViewport.MaxDepth = 1.0f;
    g_D3D10MainViewport.TopLeftX = 0;
    g_D3D10MainViewport.TopLeftY = 0;


    ID3D10RasterizerState* rs;
    D3D10_RASTERIZER_DESC rasterizerState;
    rasterizerState.FillMode = D3D10_FILL_SOLID;
    rasterizerState.CullMode = D3D10_CULL_NONE;
    rasterizerState.FrontCounterClockwise = false;
    rasterizerState.DepthBias = false;
    rasterizerState.DepthBiasClamp = 0;
    rasterizerState.SlopeScaledDepthBias = 0;
    rasterizerState.DepthClipEnable = false;
    rasterizerState.ScissorEnable = false;
    rasterizerState.MultisampleEnable = false;
    rasterizerState.AntialiasedLineEnable = false;
    g_D3D10Device->CreateRasterizerState( &rasterizerState, &rs );
    g_D3D10Device->RSSetState( rs );

    // Setup the effect
    {
        // Blit quad effect
        ID3D10Blob* pCompiledEffect;
        ID3D10Blob* pErrors = NULL;
        hr = D3D10CompileEffectFromMemory(
            (void*)g_unprojectEffectSrc,
            sizeof(g_unprojectEffectSrc),
            NULL,
            NULL, // pDefines
            NULL, // pIncludes
            0, // HLSL flags
            0, // FXFlags
            &pCompiledEffect,
            &pErrors);

        if( pErrors )
        {
            LPVOID l_pError = NULL;
            l_pError = pErrors->GetBufferPointer(); // then cast to a char* to see it in the locals window
            fprintf(stdout, "Compilation error: \n %s", (char*) l_pError);
        }

        hr = D3D10CreateEffectFromMemory(
            pCompiledEffect->GetBufferPointer(),
            pCompiledEffect->GetBufferSize(),
            0, // FXFlags
            g_D3D10Device,
            NULL,
            &g_pUnprojectEffect);
        pCompiledEffect->Release();

        g_pUnprojectTechnique = g_pUnprojectEffect->GetTechniqueByName( "Render" );

        g_pImageSizeInv = g_pUnprojectEffect->GetVariableByName("g_ImageSizeInv")->AsVector();
        D3DXVECTOR4 g_ImageSizeInv = D3DXVECTOR4( g_DXGISwapChainDesc.BufferDesc.Width, g_DXGISwapChainDesc.BufferDesc.Height,
                                      1.0 / g_DXGISwapChainDesc.BufferDesc.Width, 1.0 / g_DXGISwapChainDesc.BufferDesc.Height );
        g_pImageSizeInv->SetFloatVector( g_ImageSizeInv );

        g_pQuadRect = g_pUnprojectEffect->GetVariableByName("g_QuadRect")->AsVector();
        g_pDepthBuffer = g_pUnprojectEffect->GetVariableByName("g_DepthMap")->AsShaderResource();
        g_pDepthBuffer->SetResource( g_D3D10DepthBufferSRV );

        g_pmProjInvMatParam = g_pUnprojectEffect->GetVariableByName("g_ProjInv")->AsMatrix();
        g_pNearFarDepthProj = g_pUnprojectEffect->GetVariableByName("g_NearFarDepthProj")->AsVector();

        g_pCorrectStereo = g_pUnprojectEffect->GetVariableByName("g_CorrectStereo")->AsScalar();
        g_pCorrectStereo->SetBool( gCorrectStereo );
        g_pStereoEyeSeparation = g_pUnprojectEffect->GetVariableByName("g_StereoEyeSeparation")->AsScalar();
        g_pStereoEyeSeparation->SetFloat( g_EyeSeparation );
        g_pStereoSeparation = g_pUnprojectEffect->GetVariableByName("g_StereoSeparation")->AsScalar();
        g_pStereoConvergence = g_pUnprojectEffect->GetVariableByName("g_StereoConvergence")->AsScalar();

        g_StereoParamD3D10.createGraphics( g_D3D10Device );
        g_pStereoParamMap = g_pUnprojectEffect->GetVariableByName("g_StereoParamMap")->AsShaderResource();
        g_pStereoParamMap->SetResource( g_StereoParamD3D10.getStereoParamMapSRV() );

        // Cube effect
        hr = D3D10CompileEffectFromMemory(
            (void*)g_cubeEffectSrc,
            sizeof(g_cubeEffectSrc),
            NULL,
            NULL, // pDefines
            NULL, // pIncludes
            0, // HLSL flags
            0, // FXFlags
            &pCompiledEffect,
             &pErrors);

        if( pErrors )
        {
            LPVOID l_pError = NULL;
            l_pError = pErrors->GetBufferPointer(); // then cast to a char* to see it in the locals window
            fprintf(stdout, "Compilation error: \n %s", (char*) l_pError);
        }

        hr = D3D10CreateEffectFromMemory(
            pCompiledEffect->GetBufferPointer(),
            pCompiledEffect->GetBufferSize(),
            0, // FXFlags
            g_D3D10Device,
            NULL,
            &g_pCubeEffect);
        pCompiledEffect->Release();

        g_pCubeTechnique = g_pCubeEffect->GetTechniqueByName( "Render" );

        g_pmCubeRotParam = g_pCubeEffect->GetVariableByName("g_CubeRot")->AsMatrix();
        g_psTimeParam = g_pCubeEffect->GetVariableByName("g_Time")->AsScalar();
        g_psGridCountParam = g_pCubeEffect->GetVariableByName("g_GridCount")->AsScalar();
        g_psGridSizeParam = g_pCubeEffect->GetVariableByName("g_GridSize")->AsScalar();
        g_pmProjMatParam = g_pCubeEffect->GetVariableByName("g_ProjMat")->AsMatrix();

        // Setup projection and Unprojection matrix
        float Near = 2.f;
        float Far = 20.0f;
        float fAspectRatio = (FLOAT)g_D3D10MainViewport.Width / (FLOAT)g_D3D10MainViewport.Height;
        D3DXMATRIX mProj;
        D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI/4, fAspectRatio, Near, Far );

        D3DXMATRIX mProjInv;
        D3DXMatrixInverse( &mProjInv, 0, &mProj );

        g_pmProjMatParam->SetMatrix( mProj );
        g_pmProjInvMatParam->SetMatrix( mProjInv );

        D3DXVECTOR4 vNearFarDepthProj( Near, Far, Far / (Far - Near), -Near*Far / (Far - Near) );
        g_pNearFarDepthProj->SetFloatVector( vNearFarDepthProj );

        // Setup  no Input Layout
        g_D3D10Device->IASetInputLayout(0);
        g_D3D10Device->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    }

#endif

    return true;
}

void FreeWindowAndDevice()
{
#ifdef D3D9

    if (g_D3D9DepthBufferSurface )
    {
        g_D3D9DepthBufferSurface->Release();
        g_D3D9DepthBufferSurface = 0;
    }
    if (g_D3D9DepthBuffer )
    {
        g_D3D9DepthBuffer->Release();
        g_D3D9DepthBuffer = 0;
    }
    if (g_pCubeEffect)
    {
        g_pCubeEffect->Release(); 
        g_pCubeEffect = 0;
    }
    if (g_pUnprojectEffect)
    {
        g_pUnprojectEffect->Release();
        g_pUnprojectEffect = 0;
    }

    g_StereoParamD3D9.destroyGraphics();

    g_D3D9Device->Release();

    if (g_D3D9->Release() != 0) {
        MessageBox(NULL,_T("Failed to release all D3D objects."),_T("Test App Error"),MB_SETFOREGROUND|MB_OK|MB_SYSTEMMODAL|MB_TOPMOST);
    }

#else
    if (g_DXGISwapChain)
    {
        BOOL isFullScreen;
        g_DXGISwapChain->GetFullscreenState( &isFullScreen, 0);
        if ( isFullScreen )
        {
            g_DXGISwapChain->SetFullscreenState(FALSE, 0);
        }
    }

    if (g_pCubeEffect)
    {
        g_pCubeEffect->Release(); 
        g_pCubeEffect = 0;
    }
    if (g_pUnprojectEffect)
    {
        g_pUnprojectEffect->Release();
        g_pUnprojectEffect = 0;
    }
    if (g_D3D10DepthBufferDSV)
    {
        g_D3D10DepthBufferDSV->Release();
        g_D3D10DepthBufferDSV = 0;
    }
    if (g_D3D10DepthBufferSRV)
    {
        g_D3D10DepthBufferSRV->Release();
        g_D3D10DepthBufferSRV = 0;
    }
    if (g_D3D10BackBufferRTV)
    {
        g_D3D10BackBufferRTV->Release();
        g_D3D10BackBufferRTV = 0;
    }
    if ( g_DXGISwapChain )
    {
        g_DXGISwapChain->Release();
        g_DXGISwapChain = 0;
    }

    g_StereoParamD3D10.destroyGraphics();

    if (g_D3D10Device)
    {
        g_D3D10Device->Release();
        g_D3D10Device = 0;
    }
#endif
}


///////////////////////////////////////////////////////////////////////////////
//  Images managment
///////////////////////////////////////////////////////////////////////////////

int gCurrentCursorX = 0;
int gCurrentCursorY = 0;

float gScreenToImagePixelRateX = 1;
float gScreenToImagePixelRateY = 1;

void Render()
{
    static unsigned int frameNb = 0;

#ifdef D3D9
    HRESULT coop_lvl = g_D3D9Device->TestCooperativeLevel();

    if (coop_lvl == D3DERR_DEVICELOST) return;

    if (coop_lvl == D3DERR_DEVICENOTRESET)
    {
        g_D3D9Device->Reset(&g_D3D9PresentParams);
        DestroyWindow( g_hWnd);
        return;
    }

    g_D3D9Device->BeginScene();

    g_D3D9Device->SetDepthStencilSurface( g_D3D9DepthBufferSurface );
    g_D3D9Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF000000, 1, 0);

    g_D3D9Device->SetViewport(&g_D3D9MainViewport);

    if (frameNb > 2)
    {

        float sep, conv;
        if (NVAPI_OK != NvAPI_Stereo_GetSeparation(g_StereoHandle,&sep ))
        {
         //   MessageBoxA(NULL, "Couldn't get the separation", "NvAPI_Stereo_GetSeparation failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        }
        if (NVAPI_OK != NvAPI_Stereo_GetConvergence(g_StereoHandle,&conv ))
        {
         //   MessageBoxA(NULL, "Couldn't get the convergence", "NvAPI_Stereo_GetConvergence failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        }
        if (sep * 0.01 != g_Separation || conv != g_Convergence)
        {
            g_Separation = sep * 0.01;
            g_Convergence = conv;
            g_pUnprojectEffect->SetFloat( g_pStereoSeparation, g_Separation );
            g_pUnprojectEffect->SetFloat( g_pStereoConvergence, g_Convergence );

            g_StereoParamD3D9.updateStereoParamsMap( g_D3D9Device, g_EyeSeparation, g_Separation, g_Convergence );
        }

        // first draw the grid of boxes in color and depth buffer
        float rotSpeed = frameNb * 0.05;
        float time = frameNb*0.1;
        static float gTime = time;
        if (gAnimate)
        {
            gTime = time;
            D3DXMATRIX rot;
            D3DXMatrixRotationYawPitchRoll( &rot, 0.02 * rotSpeed, 0.01*rotSpeed, 0.05*rotSpeed);
            g_pCubeEffect->SetMatrix( g_pmCubeRotParam, &rot );
        }

        //
        // draw the cube
        //
        struct VertexStruct
        {
            float position[3];
            float texture[3];
        };

        VertexStruct VB[24] = 
        {
            {  {-1,-1,-1,},  {0,0,-1,},  }, 
            {  {1,-1,-1,},  {0,0,-1,},  },
            {  {-1,1,-1,}, {0,0,-1,},  },
            {  {1,1,-1,},  {0,0,-1,},  },

            {  {-1,1,1,}, {0,0,1,},  },
            {  {1,1,1,},  {0,0,1,},  },
            {  {-1,-1,1,},  {0,0,1,},  }, 
            {  {1,-1,1,},  {0,0,1,},  },

            {  {-1,1,-1,},  {0,1,0,},  }, 
            {  {1,1,-1,},  {0,1,0,},  },
            {  {-1,1,1,}, {0,1,0,},  },
            {  {1,1,1,},  {0,1,0,},  },

            {  { -1, -1,1,}, {0,-1,0,},  },
            {  { 1, -1,1,},  {0,-1,0,},  },
            {  {-1,-1,-1,},  {0,-1,0,},  }, 
            {  { 1,-1,-1,},  {0,-1,0,},  },

            {  {-1,-1,1,},  {-1,0,0,},  }, 
            {  {-1,-1,-1,},   {-1,0,0,},  },
            {  {-1,1,1,},   {-1,0,0,},  },
            {  {-1,1,-1,},    {-1,0,0,},  },

            {  {1,-1,-1,},   {1,0,0,},  },
            {  {1,-1,1,},    {1,0,0,},  },
            {  {1,1,-1,},  {1,0,0,},  }, 
            {  {1,1,1,},   {1,0,0,},  },
        };

        unsigned int IB[36] = 
        {
            0,1,2,
            2,1,3,

            4,5,6,
            6,5,7,

            8,9,10,
            10,9,11,

            12,13,14,
            14,13,15,

            16,17,18,
            18,17,19,

            20,21,22,
            22,21,23,
        };


        int gridCount = 11;
        float gridSize = 2.0f;

        g_D3D9Device->SetFVF(D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0));
        g_pCubeEffect->SetTechnique( g_pCubeTechnique );
        UINT cPasses;
        g_pCubeEffect->Begin( &cPasses, 0 );

            for( UINT iPass = 0; iPass < cPasses; iPass++ )
            {
                for (UINT c = 0; c < gridCount*gridCount; c++)
                {
                    {
                        float gridInvCount = 1.0f / (gridCount-1.0f);
                        float u = 2.f *gridInvCount *(c % gridCount) - 1.f;
                        float v = 2.f *gridInvCount *(c / gridCount) - 1.f;
                        float r = u*u + v*v;
                        float instanceOffset =  (1- r) * cos( r *0.5*3.14)*(sin( gTime *0.05 ));
                        D3DXVECTOR4 cubePos = D3DXVECTOR4( gridSize * ( u ), gridSize * ( v ), 5.0f + 3.0f * instanceOffset , gridSize * 0.5* gridInvCount );
                        g_pCubeEffect->SetVector( g_pvCubePosParam, &cubePos );
                    }

                    g_pCubeEffect->BeginPass( iPass );
                    g_D3D9Device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 24, 12, IB, D3DFMT_INDEX32, VB, sizeof(VertexStruct) );
                    g_pCubeEffect->EndPass();
                }
            }
        g_pCubeEffect->End();

        // Draw the full screnen quad
        // Draw the quad with unproject code

        g_D3D9Device->SetDepthStencilSurface( 0 );

        float QuadB[12] = 
        {
            0,0,0,
            1,0,0,
            0,1,0,
            1,1,0,
        };
        D3DXVECTOR4 quadRect( -0.9f, -1.0f, 1.8f , 1.0f );
        g_pUnprojectEffect->SetVector( g_pQuadRect, &quadRect);
        g_pUnprojectEffect->SetBool( g_pCorrectStereo, gCorrectStereo );

        g_D3D9Device->SetFVF(D3DFVF_XYZ);
        
        g_pUnprojectEffect->SetTechnique( g_pUnprojectTechnique );
        g_pUnprojectEffect->Begin( &cPasses, 0 );
            for( UINT iPass = 0; iPass < cPasses; iPass++ )
            {
                g_pUnprojectEffect->BeginPass( iPass );
                g_D3D9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, QuadB, sizeof(float)*3 );
                g_pUnprojectEffect->EndPass();
            }
        g_pUnprojectEffect->End();

    }

    g_D3D9Device->EndScene();
    g_D3D9Device->Present(NULL, NULL, NULL, NULL);

#else
    g_D3D10Device->RSSetViewports( 1, &g_D3D10MainViewport );

    float color[] = {0.0f, 0.0f, 0.0f, 1.f};
    g_D3D10Device->ClearRenderTargetView( g_D3D10BackBufferRTV, color);
    g_D3D10Device->ClearDepthStencilView( g_D3D10DepthBufferDSV, D3D10_CLEAR_DEPTH, 1, 0);

    g_D3D10Device->OMSetRenderTargets( 1, &g_D3D10BackBufferRTV, g_D3D10DepthBufferDSV);

    if (frameNb > 2)
    {
        float sep, conv;
        if (NVAPI_OK != NvAPI_Stereo_GetSeparation(g_StereoHandle,&sep ))
        {
         //   MessageBoxA(NULL, "Couldn't get the separation", "NvAPI_Stereo_GetSeparation failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        }
        if (NVAPI_OK != NvAPI_Stereo_GetConvergence(g_StereoHandle,&conv ))
        {
         //   MessageBoxA(NULL, "Couldn't get the convergence", "NvAPI_Stereo_GetConvergence failed", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        }
        if (sep * 0.01 != g_Separation || conv != g_Convergence)
        {
            g_Separation = sep * 0.01;
            g_Convergence = conv;
            g_pStereoSeparation->SetFloat( g_Separation );
            g_pStereoConvergence->SetFloat( g_Convergence );

            g_StereoParamD3D10.updateStereoParamsMap( g_D3D10Device, g_EyeSeparation, g_Separation, g_Convergence );
        }


        // first draw the grid of boxes in color and depth buffer
        float rotSpeed = frameNb * 0.05;
        float time = frameNb*0.1;
        if (gAnimate)
        {
            g_psTimeParam->SetFloat( time );
            D3DXMATRIX rot;
            D3DXMatrixRotationYawPitchRoll( &rot, 0.02 * rotSpeed, 0.01*rotSpeed, 0.05*rotSpeed);
            g_pmCubeRotParam->SetMatrix( rot);
        }

        int gridCount = 11;
        g_psGridCountParam->SetFloat( gridCount);
        float gridSize = 2.0f;
        g_psGridSizeParam->SetFloat( gridSize);

        g_pCubeTechnique->GetPassByIndex(0)->Apply(0);
        g_D3D10Device->Draw( 24 * gridCount*gridCount, 0 );

        // Stop drawing in the depth buffer so we can fetch from it
        g_D3D10Device->OMSetRenderTargets( 1, &g_D3D10BackBufferRTV, 0);

        // Draw the quad with unproject code
        float quadRect[4] = { -0.9f, -1.0f, 1.8f , 2.0f };
        g_pQuadRect->SetFloatVector( (float* ) &quadRect);
        g_pCorrectStereo->SetBool( gCorrectStereo );
        g_pUnprojectTechnique->GetPassByIndex(0)->Apply(0);
        g_D3D10Device->Draw( 4, 0 );
    }

    g_DXGISwapChain->Present(0, 0);

#endif

    frameNb++;

}


#define GET_MOUSECURSORX(pParam)  static_cast<int>(LOWORD(pParam))
#define GET_MOUSECURSORY(pParam)  static_cast<int>(HIWORD(pParam))
#define GET_BUTTONSTATE(pParam)  static_cast<int>(LOWORD(pParam))

static LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

    switch( msg )
    {
        case WM_MOUSEWHEEL:
        {
            int px, py;
            px = (GET_MOUSECURSORX( lParam ))*gScreenToImagePixelRateX;
            py = (GET_MOUSECURSORY( lParam ))*gScreenToImagePixelRateY;

            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (GET_BUTTONSTATE( wParam ) == MK_LBUTTON )
            {
                int dx, dy;
                dx = (gCurrentCursorX - GET_MOUSECURSORX( lParam ))*gScreenToImagePixelRateX;
                dy = (gCurrentCursorY - GET_MOUSECURSORY( lParam ))*gScreenToImagePixelRateY;
            }

            gCurrentCursorX = GET_MOUSECURSORX( lParam );
            gCurrentCursorY = GET_MOUSECURSORY( lParam );

            return 0;
        }
        case WM_MBUTTONDOWN:
        {
            int px, py;
            px = (GET_MOUSECURSORX( lParam ))*gScreenToImagePixelRateX;
            py = (GET_MOUSECURSORY( lParam ))*gScreenToImagePixelRateY;
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            int px, py;
            px = (GET_MOUSECURSORX( lParam ))*gScreenToImagePixelRateX;
            py = (GET_MOUSECURSORY( lParam ))*gScreenToImagePixelRateY;
            return 0;
        }
        case WM_KEYDOWN:
        {
            switch (wParam) {
                case VK_ESCAPE: DestroyWindow(hWnd); return 0;
                case ' ': gAnimate = !gAnimate; return 0;
                case 'C': 
                    {
                         gCorrectStereo = !gCorrectStereo;
                         return 0;
                    }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage( 0 );
            if (g_hWnd == hWnd) g_hWnd = NULL;
            return 0;

        case WM_PAINT:
            ValidateRect( hWnd, NULL );
            Render();
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    g_hInstance = hInstance;

    CreateWindowAndDevice();

    ShowWindow(g_hWnd,SW_SHOWDEFAULT);

    UpdateWindow(g_hWnd);
    UpdateWindow(g_hWnd);

    do {
        if(g_hWnd!=NULL) {
            MSG msg;
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        if(g_hWnd!=NULL) {
            Render();
        }
    } while (g_hWnd);

    FreeWindowAndDevice();

    return 0;
}
