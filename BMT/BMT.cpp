/**
* This file is part of Batman Tweak.
*
* Batman Tweak is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* The Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Batman Tweak is distributed in the hope that it will be useful,
* But WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Batman Tweak.If not, see <http://www.gnu.org/licenses/>.
**/
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"

#include "BMT.h"
#include "ini.h"
#include "xml.h"
#include "parameter.h"
#include "utility.h"
#include "nvapi.h"
#include "dxgi.h"
#include "wmi.h"

#include <map>
#include <set>

#include <cstdio>

#include <windowsx.h>
#include <CommCtrl.h> // Button_GetIdealSize

#pragma comment(lib, "Comctl32.lib")

using namespace bmt;

#define BMT_VERSION_STR L"1.01"

INT_PTR CALLBACK  Config (HWND, UINT, WPARAM, LPARAM);

bool  messagebox_active; // Workaround some particularly strange behavior
bool  first_load = true; // Some settings should only be loaded once; ignore when false

HWND  hWndApp;
HICON bmt_icon;
HICON nv_icon;

BM_Engine         engine;
BM_SystemSettings settings;

int APIENTRY _tWinMain(_In_ HINSTANCE     hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR        lpCmdLine,
                       _In_ int           nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER (nCmdShow);

  WMI::Init ();

  bmt_icon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_BMT));
  nv_icon  = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_NV));

  return (int)DialogBox (hInstance, MAKEINTRESOURCE (IDD_BATMAN), NULL, Config);
}

struct streaming_profiles {
  const wchar_t* default =
  { L"[TextureStreaming]\n"
    L"MinTextureResidentMipCount=8\n"
    L"PoolSize=1024\n"
    L"MemoryMargin=128\n"
    L"MemoryLoss=0\n"
    L"HysteresisLimit=15\n"
    L"DropMipLevelsLimit=12\n"
    L"StopIncreasingLimit=12\n"
    L"StopStreamingLimit=8\n"
    L"MinEvictSize=8\n"
    L"MinFudgeFactor=1\n"
    L"FudgeFactorIncreaseRateOfChange=0.5\n"
    L"FudgeFactorDecreaseRateOfChange=-0.4\n"
    L"MinRequestedMipsToConsider=11\n"
    L"MinTimeToGuaranteeMinMipCount=2\n"
    L"MaxTimeToGuaranteeMinMipCount=12\n"
    L"UseTextureFileCache=True\n"
    L"LoadMapTimeLimit=0\n"
    L"LightmapStreamingFactor=0.04\n"
    L"ShadowmapStreamingFactor=0.04\n"
    L"MaxLightmapRadius=2000.0\n"
    L"AllowStreamingLightmaps=False\n"
    L"TextureFileCacheBulkDataAlignment=1\n"
    L"UsePriorityStreaming=True\n"
    L"bAllowSwitchingStreamingSystem=False\n"
    L"UseDynamicStreaming=True\n"
    L"bEnableAsyncDefrag=False\n"
    L"bEnableAsyncReallocation=False\n"
    L"MaxDefragRelocations=256\n"
    L"MaxDefragDownShift=128\n"
    L"BoostPlayerTextures=3.0\n"
    L"TemporalAAMemoryReserve=4.0\n"
    L"ReflectionTexturePoolSize=72\n"
  };

  const wchar_t* slow =
  { L"[TextureStreaming]\n"
    L"AllowStreamingLightmaps=True\n"
    L"BoostPlayerTextures=4.0\n"
    L"DropMipLevelsLimit=16\n"
    L"FudgeFactorDecreaseRateOfChange=-0.4\n"
    L"FudgeFactorIncreaseRateOfChange=0.5\n"
    L"HysteresisLimit=30\n"
    L"LightmapStreamingFactor=0.05\n"
    L"LoadMapTimeLimit=3.0\n"
    L"MaxDefragDownShift=128\n"
    L"MaxDefragRelocations=256\n"
    L"MaxLightmapRadius=25000.0\n"
    L"MaxTimeToGuaranteeMinMipCount=12\n"
    L"MemoryLoss=0\n"
    L"MemoryMargin=192\n"
    L"MinEvictSize=128\n"
    L"MinFudgeFactor=1\n"
    L"MinRequestedMipsToConsider=11\n"
    L"MinTextureResidentMipCount=7\n"
    L"MinTimeToGuaranteeMinMipCount=3\n"
    L"PoolSize=1024\n"
    L"ReflectionTexturePoolSize=96\n"
    L"ShadowmapStreamingFactor=0.1\n"
    L"StopIncreasingLimit=12\n"
    L"StopStreamingLimit=8\n"
    L"TemporalAAMemoryReserve=4.0\n"
    L"TextureFileCacheBulkDataAlignment=1\n"
    L"UseDynamicStreaming=True\n"
    L"UsePriorityStreaming=True\n"
    L"UseTextureFileCache=True\n"
    L"bAllowSwitchingStreamingSystem=False\n"
    L"bEnableAsyncDefrag=False\n"
    L"bEnableAsyncReallocation=False\n"
  };

  const wchar_t* balanced =
  { L"[TextureStreaming]\n"
    L"AllowStreamingLightmaps=True\n"
    L"BoostPlayerTextures=4.0\n"
    L"DropMipLevelsLimit=14\n"
    L"FudgeFactorDecreaseRateOfChange=-0.4\n"
    L"FudgeFactorIncreaseRateOfChange=0.5\n"
    L"HysteresisLimit=19\n"
    L"LightmapStreamingFactor=0.04\n"
    L"LoadMapTimeLimit=1.0\n"
    L"MaxDefragDownShift=128\n"
    L"MaxDefragRelocations=256\n"
    L"MaxLightmapRadius=12500.0\n"
    L"MaxTimeToGuaranteeMinMipCount=10\n"
    L"MemoryLoss=0\n"
    L"MemoryMargin=192\n"
    L"MinEvictSize=64\n"
    L"MinRequestedMipsToConsider=14\n"
    L"MinTextureResidentMipCount=7\n"
    L"MinTimeToGuaranteeMinMipCount=2\n"
    L"PoolSize=4096\n"
    L"ReflectionTexturePoolSize=96\n"
    L"ShadowmapStreamingFactor=0.05\n"
    L"StopIncreasingLimit=12\n"
    L"StopStreamingLimit=8\n"
    L"TemporalAAMemoryReserve=4.0\n"
    L"TextureFileCacheBulkDataAlignment=1\n"
    L"UseDynamicStreaming=True\n"
    L"UsePriorityStreaming=True\n"
    L"UseTextureFileCache=False\n"
    /*
    L"bAllowSwitchingStreamingSystem=False\n"
    L"bEnableAsyncDefrag=False\n"
    L"bEnableAsyncReallocation=False\n"
    */
    L"bAllowSwitchingStreamingSystem=True\n"
    L"bEnableAsyncDefrag=True\n"
    L"bEnableAsyncReallocation=True\n"
  };

  const wchar_t* fast = {
    L"[TextureStreaming]\n"
    L"AllowStreamingLightmaps=True\n"
    L"BoostPlayerTextures=4.0\n"
    L"DropMipLevelsLimit=12\n"
    L"FudgeFactorDecreaseRateOfChange=-0.4\n"
    L"FudgeFactorIncreaseRateOfChange=0.5\n"
    L"HysteresisLimit=9\n"
    L"LightmapStreamingFactor=0.04\n"
    L"LoadMapTimeLimit=1\n"
    L"MaxDefragDownShift=128\n"
    L"MaxDefragRelocations=256\n"
    L"MaxLightmapRadius=2000.0\n"
    L"MaxTimeToGuaranteeMinMipCount=4\n"
    L"MemoryLoss=0\n"
    L"MemoryMargin=256\n"
    L"MinEvictSize=64\n"
    L"MinFudgeFactor=1\n"
    L"MinRequestedMipsToConsider=22\n"
    L"MinTextureResidentMipCount=6\n"
    L"MinTimeToGuaranteeMinMipCount=0\n"
    L"PoolSize=16353\n"
    L"ReflectionTexturePoolSize=96\n"
    L"ShadowmapStreamingFactor=0.04\n"
    L"StopIncreasingLimit=12\n"
    L"StopStreamingLimit=8\n"
    L"TemporalAAMemoryReserve=4.0\n"
    L"TextureFileCacheBulkDataAlignment=1\n"
    L"UseDynamicStreaming=True\n"
    L"UsePriorityStreaming=True\n"
    L"UseTextureFileCache=False\n"
    L"bAllowSwitchingStreamingSystem=False\n"
    L"bEnableAsyncDefrag=True\n"
    L"bEnableAsyncReallocation=True\n"
  };
} streaming;

struct texture_profiles {
  const wchar_t* default = {
    L"[SystemSettings]\n"
    L"TEXTUREGROUP_Bokeh=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Character=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_CharacterHigh=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_CharacterLow=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_CharacterNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_CharacterSpecPower=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_CharacterSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Cinematic=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_ColorLookupTable=(MinLODSize=-2147483648,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Effects=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_EffectsNotFiltered=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_ImageBasedReflection=(MinLODSize=128,MaxLODSize=128,LODBias=1,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_InteriorNormalMap_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_InteriorSpec_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Interior_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_LOD1_Emissive=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_Opacity_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_RRM_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LightAndShadowMap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear)\n"
    L"TEXTUREGROUP_Lightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_MobileFlattened=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=point)\n"
    L"TEXTUREGROUP_ProcBuilding_Face=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen5)\n"
    L"TEXTUREGROUP_ProcBuilding_LightMap=(MinLODSize=1,MaxLODSize=4096,LODBias=-1,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_RenderTarget=(MinLODSize=1,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Shadowmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Skybox=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Terrain_Heightmap=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Terrain_Weightmap=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_UI=(MinLODSize=512,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Vehicle=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_VehicleNormalMap=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_VehicleSpecular=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Vehicle_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_Vehicle_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_Weapon=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_WeaponNormalMap=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_WeaponSpecular=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_World=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_WorldNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_WorldNormalMap_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_WorldSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_WorldSpecular_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_World_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_World_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
  };

  const wchar_t* quality = {
#if 0
    L"[SystemSettings]\n"
    L"TEXTUREGROUP_Bokeh=(MinLODSize=128,MaxLODSize=512,LODBias=1,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Character=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterHigh=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterLow=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecPower=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Cinematic=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Effects=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_EffectsNotFiltered=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_ImageBasedReflection=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Blur5)\n"
    L"TEXTUREGROUP_InteriorNormalMap_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_InteriorSpec_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Interior_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_LOD1_Emissive=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_Opacity_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_RRM_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LightAndShadowMap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Lightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear)\n"
    L"TEXTUREGROUP_MobileFlattened=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=point)\n"
    L"TEXTUREGROUP_ProcBuilding_Face=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen5)\n"
    L"TEXTUREGROUP_ProcBuilding_LightMap=(MinLODSize=128,MaxLODSize=4096,LODBias=-1,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_RenderTarget=(MinLODSize=1,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Shadowmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,NumStreamedMips=6,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Skybox=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Terrain_Heightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Point,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Terrain_Weightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Point,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_UI=(MinLODSize=512,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_VehicleNormalMap=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_VehicleSpecular=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Weapon=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WeaponNormalMap=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WeaponSpecular=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldNormalMap_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
#else
#if 0
    // 7/13/2015 -- Changed a few texture groups that might be causing issues on AMD hardware
    L"[SystemSettings]\n"
    L"TEXTUREGROUP_Bokeh=(MinLODSize=128,MaxLODSize=512,LODBias=1,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Character=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterHigh=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterLow=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecPower=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Cinematic=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Effects=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_EffectsNotFiltered=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_ImageBasedReflection=(MinLODSize=128,MaxLODSize=128,LODBias=1,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    //L"TEXTUREGROUP_ImageBasedReflection=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Blur5)\n"
    L"TEXTUREGROUP_InteriorNormalMap_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_InteriorSpec_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Interior_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_LOD1_Emissive=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_Opacity_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_RRM_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LightAndShadowMap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Lightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_MobileFlattened=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=point)\n"
    L"TEXTUREGROUP_ProcBuilding_Face=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen5)\n"
    L"TEXTUREGROUP_ProcBuilding_LightMap=(MinLODSize=128,MaxLODSize=4096,LODBias=-1,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_RenderTarget=(MinLODSize=1,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Shadowmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Skybox=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Terrain_Heightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Terrain_Weightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_UI=(MinLODSize=512,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_VehicleNormalMap=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_VehicleSpecular=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Weapon=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WeaponNormalMap=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WeaponSpecular=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldNormalMap_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
#else
    L"[SystemSettings]\n"
    L"TEXTUREGROUP_Bokeh=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Character=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterHigh=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterLow=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecPower=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_CharacterSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Cinematic=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_ColorLookupTable=(MinLODSize=-2147483648,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Effects=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_EffectsNotFiltered=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_ImageBasedReflection=(MinLODSize=128,MaxLODSize=128,LODBias=1,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_InteriorNormalMap_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_InteriorSpec_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Interior_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_LOD1_Emissive=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_Opacity_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LOD1_RRM_Mask=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_LightAndShadowMap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear)\n"
    L"TEXTUREGROUP_Lightmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_MobileFlattened=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=point)\n"
    L"TEXTUREGROUP_ProcBuilding_Face=(MinLODSize=256,MaxLODSize=4096,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen5)\n"
    L"TEXTUREGROUP_ProcBuilding_LightMap=(MinLODSize=1,MaxLODSize=4096,LODBias=-1,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_RenderTarget=(MinLODSize=1,MaxLODSize=2048,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Shadowmap=(MinLODSize=128,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Skybox=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Terrain_Heightmap=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Terrain_Weightmap=(MinLODSize=1,MaxLODSize=4096,LODBias=0,MinMagFilter=Aniso,MipFilter=Point,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_UI=(MinLODSize=512,MaxLODSize=4096,LODBias=0,MinMagFilter=Linear,MipFilter=Linear,MipGenSettings=TMGS_SimpleAverage)\n"
    L"TEXTUREGROUP_Vehicle=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_VehicleNormalMap=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_VehicleSpecular=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_Vehicle_High=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_Vehicle_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_Weapon=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_WeaponNormalMap=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WeaponSpecular=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_WorldNormalMap=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldNormalMap_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular=(MinLODSize=128,MaxLODSize=1024,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_WorldSpecular_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_LeaveExistingMips)\n"
    L"TEXTUREGROUP_World_Hi=(MinLODSize=128,MaxLODSize=2048,LODBias=0,MinMagFilter=aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
    L"TEXTUREGROUP_World_Low=(MinLODSize=128,MaxLODSize=512,LODBias=0,MinMagFilter=Aniso,MipFilter=Linear,MipGenSettings=TMGS_Sharpen6)\n"
#endif
#endif
  };
} texgroups;

struct mip_fadein_profiles {
  const wchar_t* default = {
    L"[Engine.Engine]\n"
    L"MipFadeInSpeed0=0.3\n"
    L"MipFadeInSpeed1=2.0\n"
  };

  const wchar_t* instant = {
    L"[Engine.Engine]\n"
    L"MipFadeInSpeed0=0.0\n"
    L"MipFadeInSpeed1=0.0\n"
  };
} fadein;

struct mip_fadeout_profiles {
  const wchar_t* default = {
    L"[Engine.Engine]\n"
    L"MipFadeOutSpeed0=0.1\n"
    L"MipFadeOutSpeed1=1.0\n"
  };

  const wchar_t* instant {
    L"[Engine.Engine]\n"
    L"MipFadeOutSpeed0=0.0\n"
    L"MipFadeOutSpeed1=0.0\n"
  };
} fadeout;


// Add a few fields to the INI files that only pertain to this
//   program.
struct bmt_signatures {
  const wchar_t* engine = {
    L"[BMT.User]\n"
    L"TweakedBy=" BMT_VERSION_STR L"\n"
  };

  const wchar_t* settings = {
    L"[BMT.User]\n"
    L"TweakedBy=" BMT_VERSION_STR L"\n"
  };
} signatures;


void tune_physx_memory (HWND hDlg, size_t& heap, size_t& mesh_cache)
{
  DXGI_ADAPTER_DESC* nv_adapters = NVAPI::EnumGPUs_DXGI ();

  if (NVAPI::CountPhysicalGPUs () > 0) {
    int cur_sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_PHYSX_GPU));

    if (cur_sel > 0) {
      heap = 0.25 * nv_adapters [cur_sel].DedicatedVideoMemory / 1024 / 1024;
    }
    else {
      heap = min (nv_adapters [cur_sel].DedicatedVideoMemory / 1024 / 1024, 128);
    }

    mesh_cache = BMT_NextPowerOfTwo (heap / 4);
  }
}


void get_resolution (HWND hDlg, int* x, int* y, int* refresh)
{
  HWND hWndResolution = GetDlgItem (hDlg, IDC_RESOLUTION);
  HWND hWndRefresh    = GetDlgItem (hDlg, IDC_REFRESH_RATE);

  wchar_t wszRes [64];
  ComboBox_GetText (hWndResolution, wszRes, 64);

  wchar_t wszRef [16];
  ComboBox_GetText (hWndRefresh, wszRef, 16);

  swscanf_s (wszRes, L"%dx%d", x, y);
  swscanf_s (wszRef, L"%d", refresh);
}

void setup_resolution (HWND hDlg)
{
  DEVMODE dmDisplay;
  ZeroMemory (&dmDisplay, sizeof DEVMODE);
  dmDisplay.dmSize = sizeof DEVMODE;

  // X,Y pairs are mapped to sets of refresh rates
  std::map <std::pair <int, int>, std::set <int> > resolutions;

  int i = 0;
  while (EnumDisplaySettings (NULL, i, &dmDisplay)) {
    i++;

    std::pair <int, int> res;
    res.first  = dmDisplay.dmPelsWidth;
    res.second = dmDisplay.dmPelsHeight;
    resolutions [res].insert (dmDisplay.dmDisplayFrequency);
  }

  std::map <std::pair <int, int>, std::set <int> >::const_iterator res = resolutions.begin ();
  std::map <std::pair <int, int>, std::set <int> >::const_iterator end = resolutions.end ();

  HWND hWndResolution = GetDlgItem (hDlg, IDC_RESOLUTION);
  HWND hWndRefresh    = GetDlgItem (hDlg, IDC_REFRESH_RATE);

  ComboBox_ResetContent (hWndResolution);
  ComboBox_ResetContent (hWndRefresh);

  int sel = -1;
        i =  0;
  while (res != end) {
    wchar_t wszRes [64];
    swprintf (wszRes, 64, L"%dx%d", res->first.first, res->first.second);
    ComboBox_InsertString (hWndResolution, i++, wszRes);
    if (res_x->get_value () == res->first.first &&
        res_y->get_value () == res->first.second)
      sel = (i - 1);
    ++res;
  }

  // Set to highest resolution if no valid resolution is selected
  if (sel == -1)
    sel = (i - 1);

  ComboBox_SetCurSel (hWndResolution, sel);

  res = resolutions.find (std::pair <int, int> (res_x->get_value (), res_y->get_value ()));

  if (res != resolutions.end ()) {
    std::set <int>::const_iterator refresh = res->second.begin ();

      i =  0;
    sel = -1;

    while (refresh != res->second.end ()) {
      wchar_t wszRefresh [64];
      swprintf (wszRefresh, 64, L"%d", *refresh);
      ComboBox_InsertString (hWndRefresh, i++, wszRefresh);
      if (refresh_rate->get_value () == *refresh)
        sel = (i - 1);
      ++refresh;
    }

    // Set to highest refresh if no valid refresh is selected
    if (sel == -1)
      sel = (i - 1);

    ComboBox_SetCurSel (hWndRefresh, sel);
  }
}

// This resizes a UI control, avoid calling it multiple times or it will continue
//   to shrink each time!
void setup_driver_tweaks (HWND hDlg)
{
  // If there is an NV GPU installed, display a special button!
  if (NVAPI::CountPhysicalGPUs () > 0) {
    std::wstring button_label = L"    NVIDIA Driver Tweaks\r\n    (Version: ";
    button_label += NVAPI::GetDriverVersion () + L")";

    SetWindowText (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), button_label.c_str ());
    ShowWindow (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), true);

    // Resize the GPU Info Text Window to fit the new button...
    RECT wnd_rect;
    GetWindowRect (GetDlgItem (hDlg, IDC_GPUINFO), &wnd_rect);
    SetWindowPos (GetDlgItem (hDlg, IDC_GPUINFO), NULL, 0, 0, wnd_rect.right - wnd_rect.left, wnd_rect.bottom - wnd_rect.top - 40, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SendMessage  (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), BM_SETIMAGE, IMAGE_ICON, LPARAM (nv_icon));
  }
}

void setup_physx_properties (HWND hDlg)
{
  DXGI_ADAPTER_DESC* nv_adapters      = NVAPI::EnumGPUs_DXGI     ();
  static int         physical_nv_gpus = NVAPI::CountPhysicalGPUs ();

  if (physical_nv_gpus > 0) {
    Button_Enable (GetDlgItem (hDlg, IDC_HARDWARE_PHYSX), true);
  }
  else {
    Button_Enable (GetDlgItem (hDlg, IDC_HARDWARE_PHYSX), false);
    hardware_physx->set_value (true);
  }

  Button_Enable (GetDlgItem (hDlg, IDC_INTERACTIVE_SMOKE),  !hardware_physx->get_value ());
  Button_Enable (GetDlgItem (hDlg, IDC_INTERACTIVE_DEBRIS), !hardware_physx->get_value ());

  if (hardware_physx->get_value ()) {
    interactive_debris->set_value (false);
    interactive_smoke->set_value  (false);
  }

  Button_Enable   (GetDlgItem (hDlg, IDC_HIGH_PHYSX),    false);
  Button_Enable   (GetDlgItem (hDlg, IDC_ULTRA_PHYSX),   false);
  ComboBox_Enable (GetDlgItem (hDlg, IDC_PHYSX_GPU),     false);
  Button_Enable   (GetDlgItem (hDlg, IDC_PHYSX_MEMTUNE), false);

  if (! hardware_physx->get_value ()) {
    Button_Enable (GetDlgItem (hDlg, IDC_HIGH_PHYSX), true);

    int cur_sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_PHYSX_GPU));

    if (cur_sel < 0)
      cur_sel = physical_nv_gpus - 1;

    ComboBox_ResetContent (GetDlgItem (hDlg, IDC_PHYSX_GPU));
    for (int i = 0; i < physical_nv_gpus; i++) {
      ComboBox_InsertString (GetDlgItem (hDlg, IDC_PHYSX_GPU), i, nv_adapters [i].Description);
    }
    ComboBox_SetCurSel (GetDlgItem (hDlg, IDC_PHYSX_GPU), cur_sel);

    if (physical_nv_gpus > 1) {
      if (ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_PHYSX_GPU)) > 0)
        Button_Enable (GetDlgItem (hDlg, IDC_ULTRA_PHYSX), true);
      else
        Button_Enable (GetDlgItem (hDlg, IDC_ULTRA_PHYSX), false);

      ComboBox_Enable (GetDlgItem (hDlg, IDC_PHYSX_GPU),     true);
      Button_Enable   (GetDlgItem (hDlg, IDC_PHYSX_MEMTUNE), true);
    }
  }
}

int get_level_of_detail (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_LEVEL_OF_DETAIL));

  if (sel == 0)
    return 0;
  else if (sel == 1)
    return 1;
  else
    return 2;
}

void setup_shadow_quality (HWND hDlg)
{
  HWND hWndShadowQuality = GetDlgItem (hDlg, IDC_SHADOW_QUALITY);

  ComboBox_ResetContent (hWndShadowQuality);
  ComboBox_InsertString (hWndShadowQuality, 0, L"Low");
  ComboBox_InsertString (hWndShadowQuality, 1, L"Normal");
  ComboBox_InsertString (hWndShadowQuality, 2, L"High");

  switch (shadow_quality->get_value ())
  {
    case 0:
      ComboBox_SetCurSel (hWndShadowQuality, 0);
      break;
    default:
    case 1:
      ComboBox_SetCurSel (hWndShadowQuality, 1);
      break;
    case 2:
      ComboBox_SetCurSel (hWndShadowQuality, 2);
      break;
  }
}

int get_shadow_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_SHADOW_QUALITY));

  if (sel == 0)
    return 0;
  else if (sel == 1)
    return 1;
  else
    return 2;
}

void setup_level_of_detail (HWND hDlg)
{
  HWND hWnd_LOD = GetDlgItem (hDlg, IDC_LEVEL_OF_DETAIL);

  ComboBox_ResetContent (hWnd_LOD);
  ComboBox_InsertString (hWnd_LOD, 0, L"Low");
  ComboBox_InsertString (hWnd_LOD, 1, L"Normal");
  ComboBox_InsertString (hWnd_LOD, 2, L"High");

  switch (level_of_detail->get_value ())
  {
    case 0:
      ComboBox_SetCurSel (hWnd_LOD, 0);
      break;
    case 1:
      ComboBox_SetCurSel (hWnd_LOD, 1);
      break;
    case 2:
    default:
      ComboBox_SetCurSel (hWnd_LOD, 2);
      break;
  }
}

int get_blur_samples (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_COMBO3));

  if (sel == 0)
    return 2;
  else if (sel == 1)
    return 4;
  else
    return 16;
}

void setup_blur_samples (HWND hDlg)
{
  HWND hWndBlurSamples = GetDlgItem (hDlg, IDC_COMBO3);

  ComboBox_ResetContent (hWndBlurSamples);
  ComboBox_InsertString (hWndBlurSamples, 0, L"2 (Low Quality)");
  ComboBox_InsertString (hWndBlurSamples, 1, L"4 (Medium Quality)");
  ComboBox_InsertString (hWndBlurSamples, 2, L"16 (High Quality)");

  if (blur_samples->get_value () == 2)
    ComboBox_SetCurSel (hWndBlurSamples, 0);
  else if (blur_samples->get_value () == 4)
    ComboBox_SetCurSel (hWndBlurSamples, 1);
  else
    ComboBox_SetCurSel (hWndBlurSamples, 2);
}

void setup_tex_filter (HWND hDlg)
{
  HWND hWndAnisotropy = GetDlgItem (hDlg, IDC_ANISOTROPY);

  ComboBox_ResetContent (hWndAnisotropy);
  ComboBox_InsertString (hWndAnisotropy, 0, L"Trilinear");
  ComboBox_InsertString (hWndAnisotropy, 1, L"2x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 2, L"4x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 3, L"8x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 4, L"16x Anisotropic");

  switch (anisotropy->get_value ()) {
    case 1:
      // 1 sample = Bilinear + Linear/Point filter on Mip
      ComboBox_SetCurSel (hWndAnisotropy, 0);
      break;
    case 2:
      // 2X AF
      ComboBox_SetCurSel (hWndAnisotropy, 1);
      break;

      /// Believe it or not, anisotropy doesn't have to be a power-of-two
      ///   or even an integer... but the Unreal engine might not accept
      ///     those sorts of values, so let's not confuse it ;)

    default: // Always fallback to 4x AF if bad values are passed.
    case 4:  // 4x AF
      ComboBox_SetCurSel (hWndAnisotropy, 2);
      break;
    case 8: // 8x AF
      ComboBox_SetCurSel (hWndAnisotropy, 3);
      break;
    case 16: // 16x AF
      ComboBox_SetCurSel (hWndAnisotropy, 4);
      break;
  }
}

int get_tex_filter (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_ANISOTROPY));

  if (sel == 0)
    return 1;
  else if (sel == 1)
    return 2;
  else if (sel == 2)
    return 4;
  else if (sel == 3)
    return 8;
  else
    return 16;
}

void setup_tex_res (HWND hDlg)
{
  HWND hWndTexRes = GetDlgItem (hDlg, IDC_TEXTURE_RES);

  ComboBox_ResetContent (hWndTexRes);
  ComboBox_InsertString (hWndTexRes, 0, L"Low (128x128 - 512x512) - 2 GiB VRAM");
  ComboBox_InsertString (hWndTexRes, 1, L"Normal (128x128 - 1024x1024) - 4 GiB VRAM");
  ComboBox_InsertString (hWndTexRes, 2, L"High (128x128 - 2048x2048) - 6 GiB VRAM");

  if (texture_res->get_value () == 2)      // High
    ComboBox_SetCurSel (hWndTexRes, 2);
  else if (texture_res->get_value () == 1) // Med
    ComboBox_SetCurSel (hWndTexRes, 1);
  else
    ComboBox_SetCurSel (hWndTexRes, 0);    // Anything Else = Low
}

int get_tex_res (HWND hDlg)
{
  return ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_TEXTURE_RES));
}

void setup_shadow_scale (HWND hDlg)
{
  HWND hWndShadowScale = GetDlgItem (hDlg, IDC_SHADOW_SCALE);

  ComboBox_ResetContent (hWndShadowScale);
  ComboBox_InsertString (hWndShadowScale, 0, L"0.25 (Ideal: 4K)");
  ComboBox_InsertString (hWndShadowScale, 1, L"0.666 (Ideal: 1440p)");
  ComboBox_InsertString (hWndShadowScale, 2, L"1.0 (Ideal: 1080p)");
  ComboBox_InsertString (hWndShadowScale, 3, L"2.0 (High Quality)");
  ComboBox_InsertString (hWndShadowScale, 4, L"4.0 (Ultra Quality)");

  const float shadow_scale_val = shadow_scale->get_value ();

  if (shadow_scale_val <= 0.25f)
    ComboBox_SetCurSel (hWndShadowScale, 0);
  else if (shadow_scale_val <= 0.6667f)
    ComboBox_SetCurSel (hWndShadowScale, 1);
  else if (shadow_scale_val <= 1.0f)
    ComboBox_SetCurSel (hWndShadowScale, 2);
  else if (shadow_scale_val <= 2.0f)
    ComboBox_SetCurSel (hWndShadowScale, 3);
  else if (shadow_scale_val <= 4.0f)
    ComboBox_SetCurSel (hWndShadowScale, 4);

  // Sane fallback
  else
  {
    shadow_scale->set_value (1.0f);
    setup_shadow_scale      (hDlg);
  }
}

float get_shadow_scale (HWND hDlg)
{
  int cur_sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_SHADOW_SCALE));

  if (cur_sel == 0)
    return 0.25f;
  if (cur_sel == 1)
    return 0.666f;
  if (cur_sel == 2)
    return 1.0f;
  if (cur_sel == 3)
    return 2.0f;
  if (cur_sel == 4)
    return 4.0f;

  // Sane fallback for invalid selection
  return 1.0f;
}

void setup_fadein (HWND hDlg)
{
  HWND hWndFadeInDefault = GetDlgItem (hDlg, IDC_FADEIN_DEFAULT),
       hWndFadeInInstant = GetDlgItem (hDlg, IDC_FADEIN_INSTANT);

  bool default = (mip_fadein0->get_value () == 0.3f &&
                  mip_fadein1->get_value () == 2.0f);
  bool instant = (! default);

  Button_SetCheck (hWndFadeInDefault, default);
  Button_SetCheck (hWndFadeInInstant, instant);
}

void store_fadein (HWND hDlg)
{
  if (Button_GetCheck (GetDlgItem (hDlg, IDC_FADEIN_DEFAULT)))
    engine.import (fadein.default);
  else
    engine.import (fadein.instant);
}

void setup_fadeout (HWND hDlg)
{
  HWND hWndFadeOutDefault = GetDlgItem (hDlg, IDC_FADEOUT_DEFAULT),
       hWndFadeOutInstant = GetDlgItem (hDlg, IDC_FADEOUT_INSTANT);

  bool default = (mip_fadeout0->get_value () == 0.1f &&
                  mip_fadeout1->get_value () == 1.0f);
  bool instant = (! default);

  Button_SetCheck (hWndFadeOutDefault, default);
  Button_SetCheck (hWndFadeOutInstant, instant);
}

void store_fadeout (HWND hDlg)
{
  if (Button_GetCheck (GetDlgItem (hDlg, IDC_FADEOUT_DEFAULT)))
    engine.import (fadeout.default);
  else
    engine.import (fadeout.instant);
}


void setup_framerate_limiting (HWND hDlg)
{
  HWND hWndFrameRateLimiting = GetDlgItem (hDlg, IDC_FRAME_RATE_LIMITING);

  EnableWindow (hWndFrameRateLimiting, FALSE);

  ComboBox_ResetContent (hWndFrameRateLimiting);
  ComboBox_InsertString (hWndFrameRateLimiting, 0, L"0   (All Bits Off)");  // 00000000
  ComboBox_InsertString (hWndFrameRateLimiting, 1, L"7   (Default)");       // 00000111
  ComboBox_InsertString (hWndFrameRateLimiting, 2, L"14  (Double Default)");// 00001110
  ComboBox_InsertString (hWndFrameRateLimiting, 3, L"15  (Low 4 Bits On)"); // 00001111
  ComboBox_InsertString (hWndFrameRateLimiting, 4, L"255 (All Bits On)");   // 11111111

  switch (framerate_limiting->get_value ())
  {
    case 0:
      ComboBox_SetCurSel (hWndFrameRateLimiting, 0);
      break;
    default:
    case 7:
      ComboBox_SetCurSel (hWndFrameRateLimiting, 1);
      break;
    case 14:
      ComboBox_SetCurSel (hWndFrameRateLimiting, 2);
      break;
    case 15:
      ComboBox_SetCurSel (hWndFrameRateLimiting, 3);
      break;
    case 255:
      ComboBox_SetCurSel (hWndFrameRateLimiting, 4);
      break;
  }
}

int get_framerate_limiting (HWND hDlg)
{
  int cur_sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_FRAME_RATE_LIMITING));

  if (cur_sel == 0)
    return 0;

  if (cur_sel == 1)
    return 7;

  if (cur_sel == 2)
    return 14;

  if (cur_sel == 3)
    return 15;

  if (cur_sel == 4)
    return 255;

  return 7;
}

bool BMT_EpsilonTest (float in, float test)
{
  const float eps = 0.0001f;

  if (in + eps > test && in - eps < test)
    return true;

  return false;
}

void setup_max_delta_time (HWND hDlg)
{
  HWND hWndMaxDeltaTime = GetDlgItem (hDlg, IDC_MAX_DELTA_TIME);

  ComboBox_ResetContent (hWndMaxDeltaTime);
  ComboBox_InsertString (hWndMaxDeltaTime, 0, L"Default (0.0416666)");
  ComboBox_InsertString (hWndMaxDeltaTime, 1, L"Alternate #1 (0.0624999)");
  ComboBox_InsertString (hWndMaxDeltaTime, 2, L"Alternate #2 (0.0833332)");
  ComboBox_InsertString (hWndMaxDeltaTime, 3, L" * Test Only (0.0)");

  const float epsilon  = 0.0001f;
  const float test_val = max_delta_time->get_value ();

  if (BMT_EpsilonTest (0.041666f, test_val))
    ComboBox_SetCurSel (hWndMaxDeltaTime, 0);
  else if (BMT_EpsilonTest (0.0624999f, test_val))
    ComboBox_SetCurSel (hWndMaxDeltaTime, 1);
  else if (BMT_EpsilonTest (0.0833332f, test_val))
    ComboBox_SetCurSel (hWndMaxDeltaTime, 2);
  else if (BMT_EpsilonTest (0.0000f, test_val))
    ComboBox_SetCurSel (hWndMaxDeltaTime, 3);

  // Sane default
  else
    ComboBox_SetCurSel (hWndMaxDeltaTime, 0);
}

float get_max_delta_time (HWND hDlg)
{
  HWND hWndMaxDeltaTime = GetDlgItem (hDlg, IDC_MAX_DELTA_TIME);

  switch (ComboBox_GetCurSel (hWndMaxDeltaTime))
  {
    default:
    case 0:
      return 0.0416666f;
    case 1:
      return 0.0624999f;
    case 2:
      return 0.0833332f;
    case 3:
      return 0.0f;
  }
}

void setup_visibility_frames (HWND hDlg)
{
  HWND hWndVisibility = GetDlgItem (hDlg, IDC_VISIBILITY);

  EnableWindow (hWndVisibility, FALSE);

  ComboBox_ResetContent (hWndVisibility);
  ComboBox_InsertString (hWndVisibility, 0, L"* Disable");
  ComboBox_InsertString (hWndVisibility, 1, L"* 1 Frame");
  ComboBox_InsertString (hWndVisibility, 2, L"7 Frames");
  ComboBox_InsertString (hWndVisibility, 3, L"8 Frames (Default)");
  ComboBox_InsertString (hWndVisibility, 4, L"9 Frames");
  ComboBox_InsertString (hWndVisibility, 5, L"* 60 Frames");

  switch (visibility_frames->get_value ())
  {
    case 0:
      ComboBox_SetCurSel (hWndVisibility, 0);
      break;
    case 1:
      ComboBox_SetCurSel (hWndVisibility, 1);
      break;
    case 7:
      ComboBox_SetCurSel (hWndVisibility, 2);
      break;
    default:
    case 8:
      ComboBox_SetCurSel (hWndVisibility, 3);
      break;
    case 9:
      ComboBox_SetCurSel (hWndVisibility, 4);
      break;
    case 60:
      ComboBox_SetCurSel (hWndVisibility, 5);
      break;
  }
}

int get_visibility_frames (HWND hDlg)
{
  int cur_sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_VISIBILITY));

  switch (cur_sel)
  {
    case 0:
      return 0;
    case 1:
      return 1;
    case 2:
      return 7;
    default:
    case 3:
      return 8;
    case 4:
      return 9;
    case 5:
      return 60;
  }
}

void
BMT_SetStreamingPoolSize (void)
{ 
  wchar_t wszPoolSize [32];

  // Convert from Bytes to MiB (Reserving 100%)
  _ui64tow (DXGI::GetGART () / 1024 / 1024, wszPoolSize, 10);

  engine.set_value (L"TextureStreaming", L"PoolSize", wszPoolSize);
}

void
BMT_SetStreamingMargin (void)
{
  wchar_t wszMargin [32];

  // Convert from Bytes to MiB (Reserving 5%)
  _ui64tow (BMT_NextPowerOfTwo (size_t ((DXGI::GetVRAM () / 1024 / 1024) * 0.05f)), wszMargin, 10);

  // Bytes to leave free
  engine.set_value (L"TextureStreaming", L"MemoryMargin", wszMargin);
}

void
BMT_SetStreamingEvictSize (void)
{
  wchar_t wszEvict [32];

  // Convert from Bytes to MiB (Reserving 1.25%)
  _ui64tow (max (BMT_NextPowerOfTwo (size_t ((DXGI::GetVRAM () / 1024 / 1024) * 0.0125f)), 16), wszEvict, 10);

  // Bytes to free when allocation fails
  engine.set_value (L"TextureStreaming", L"MinEvictSize", wszEvict);
}

void
BMT_OptimizeStreamingMemory (void)
{
  BMT_SetStreamingPoolSize  ();
  BMT_SetStreamingMargin    ();
  BMT_SetStreamingEvictSize ();
}

INT_PTR CALLBACK DriverConfigNV (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



// What a nasty kludge this is, fix this ASAP!!!!
int mode = 0;

void handle_window_radios (HWND hDlg, WORD ID)
{
  switch (ID) {
    case IDC_BORDER_WINDOW:
      mode = 0;
      break;
    case IDC_BORDERLESS_WINDOW:
      if (mode == 1)
        mode = 3;
      else
        mode = 1;
      break;
    case IDC_FULLSCREEN:
      mode = 2;
      break;
  }

  if (mode < 3) {
    LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
         style = (style & ~BS_AUTO3STATE) | BS_AUTORADIOBUTTON;
    SetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
  }

  if (mode == 0) {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW),     1);
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),        0);
  }
  else if (mode == 2) {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW),     0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),        1);
  }
  else {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),    0);

    // Regular borderless
    if (mode == 1) {
      LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
           style = (style & ~BS_AUTO3STATE) | BS_AUTORADIOBUTTON;
      SetWindowLong   (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
      Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 1);

      // Ignore BMAK startup/shutdown, we're not changing resolution
      WMI::StopMonitoring ();
    }
    else {
      LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
           style = (style & ~BS_AUTORADIOBUTTON) | BS_AUTO3STATE;
      SetWindowLong   (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
      Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 2);

      // Monitor BMAK startup/shutdown to change the desktop res...
      WMI::StartMonitoring ();
    }
  }

  // Only enable this button in fullscreen mode...
  Button_Enable (GetDlgItem (hDlg, IDC_VSYNC), (mode == 2));

  if (mode != 2)
    Button_SetCheck (GetDlgItem (hDlg, IDC_VSYNC), 0);
  else
    Button_SetCheck (GetDlgItem (hDlg, IDC_VSYNC), use_vsync->get_value ());
}

void setup_debug_utils (HWND hDlg, bool debug)
{
  ShowWindow (GetDlgItem (hDlg, IDC_GPUINFO),          (! debug));
  ShowWindow (GetDlgItem (hDlg, IDC_GPU_GROUP),        (! debug));
  if (NVAPI::CountPhysicalGPUs () > 0) {
    // Show/hide the NVIDIA driver tweaks button if applicable

    ShowWindow (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), (!debug));
  }
  ShowWindow (GetDlgItem (hDlg, IDC_DEBUG_GROUP),         debug);
  ShowWindow (GetDlgItem (hDlg, IDC_NUKE_CONFIG),         debug);
  ShowWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),       debug);
  ShowWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG),      debug);

  if (debug) {
    if (BMT_HasBackupConfigFiles ()) {
      std::wstring backup_time = BMT_GetBackupFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), std::wstring (L" Restore Config Files\n " +
                                                                            backup_time).c_str ());
      EnableWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), TRUE);
      //EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), FALSE);
      EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),  TRUE);
      std::wstring config_time = BMT_GetConfigFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), std::wstring (L" Backup Config Files\n " +
                                                                         config_time).c_str ());
    }
    else {
      std::wstring config_time = BMT_GetConfigFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), std::wstring (L" Backup Config Files\n " +
                                                                         config_time).c_str ());
      EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),  TRUE);
      EnableWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), FALSE);
      SetWindowText (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), L" Restore Config Files");
    }
  }
}


using namespace bmt;
using namespace bmt::UI;
using namespace bmt::XML;

INT_PTR
CALLBACK
Config (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      SendMessage (hDlg, WM_SETICON, ICON_BIG,   (LPARAM)bmt_icon);
      SendMessage (hDlg, WM_SETICON, ICON_SMALL, (LPARAM)bmt_icon);

      // Wow this code is ugly, it all needs to be wrapped...
      HINSTANCE hShell32 = LoadLibrary (L"shell32.dll");
      HICON     hIcon    = LoadIcon    (hShell32, MAKEINTRESOURCE (16761));

      SIZE size;
      SendMessage (GetDlgItem (hDlg, IDOK), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
      Button_GetIdealSize (GetDlgItem (hDlg, IDOK), &size);
      SetWindowPos (GetDlgItem (hDlg, IDOK), NULL, 0, 0, size.cx + 6, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (200));

      SendMessage (GetDlgItem (hDlg, IDCANCEL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
      Button_GetIdealSize (GetDlgItem (hDlg, IDCANCEL), &size);
      SetWindowPos (GetDlgItem (hDlg, IDCANCEL), NULL, 0, 0, size.cx + 6, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (145));
      SendMessage (GetDlgItem (hDlg, IDC_NUKE_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (16771));
      SendMessage (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (16741));
      SendMessage (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      FreeLibrary (hShell32);

      SetWindowText (hDlg, L"Batman Tweak v " BMT_VERSION_STR);

      hWndApp = hDlg;

      // Don't do this stuff multiple times, that'd be bad!
      if (first_load)
        WMI::Install (); // Cleanup is automatic thanks to an `atexit` hack, consider changing this policy...

      NVAPI::InitializeLibrary ();

      if (! LoadXML ()) {
        EndDialog (hDlg, FALSE);
        NVAPI::UnloadLibrary ();
        return 0;
      }

      settings.load (install_path);
      engine.load   (install_path);

      // Setup the config variables on first load only
      if (first_load) {
        // Add the signatures at load-time, instead of save-time... this might improve
        //  some reports of weird behavior. 
        settings.import (signatures.settings);
        engine.import   (signatures.engine);

        refresh_rate =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Refresh Rate")
          );

        res_x =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"X Resolution")
          );

        res_y =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Y Resolution")
          );

        max_fps =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Maximum Framerate")
          );

        use_vsync =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Use VSYNC")
          );

        smooth_framerate =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Framerate Smoothing")
          );

        smoothed_min =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Minimum Smoothed Range")
          );

        smoothed_max =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Maximum Smoothed Range")
          );

        blur_samples =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Number of Blur Filter Samples")
          );

        // In some APIs this is a float, but let's just keep things simple (int).
        anisotropy =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Maximum Anisotropic Filter")
          );

        texture_res =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Texture Resolution Level")
          );

        hardware_physx =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Disable Hardware PhysX")
          );

        physx_level =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"PhysX Level")
          );

        physx_heap_size =
          static_cast <ParameterInt64 *> (
            g_ParameterFactory.create_parameter <int64_t> (L"PhysX Heap Size (GPU)")
          );

        physx_mesh_cache =
          static_cast <ParameterInt64 *> (
            g_ParameterFactory.create_parameter <int64_t> (L"PhysX Mesh Cache (GPU)")
          );

        enable_dx10 =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Enable DX10 Features")
          );

        // This is probably pure fantasy, but whatever... some people insist on having this.
        enable_dx11 =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Enable DX11 Features")
          );

        enable_crossfire =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Enable CrossFire")
          );

        level_of_detail =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Level of Detail")
          );

        shadow_quality =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Shadow Quality")
          );

        antialiasing =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Antialiasing")
          );

        interactive_debris =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Interactive Paper Debris")
          );

        interactive_smoke =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Interactive Smoke")
          );

        enhanced_rain =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Interactive Smoke")
          );

        enhanced_lightshafts =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"Volumetric Lighting (Enhanced Lightshafts)")
          );

        mip_fadein0 =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Mipmap LOD0 FadeIn Rate")
          );
        mip_fadein1 =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Mipmap LOD1 FadeIn Rate")
          );

        mip_fadeout0 =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Mipmap LOD0 FadeOut Rate")
          );
        mip_fadeout1 =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Mipmap LOD1 FadeOut Rate")
          );

        shadow_scale =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Shadow Scale (Shadow Texels Per-Pixel)")
          );

        framerate_limiting =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Frame Rate Limiting Setting (not what it sounds like)")
          );

        max_delta_time =
          static_cast <ParameterFloat *> (
            g_ParameterFactory.create_parameter <float> (L"Maximum Delta Time")
          );

        visibility_frames =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Primitive Probably Visible Time")
          );

        // User-Defined Configuration -- NOT PART OF THE GAME
        streaming_profile =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"User-Selected Streaming Profile")
          );

        texgroup_profile =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"User-Selected TexGroup Profile")
          );
      }

      refresh_rate->register_to_xml (FindNode (bmak_gamesettings, L"RESOLUTION"), L"RefreshRate");
      refresh_rate->register_to_ini (engine.get_file (), L"Engine.Client", L"MinDesiredFrameRate");
      refresh_rate->load ();

      res_x->register_to_xml (FindOption (bmak_gamesettings, L"ResolutionX"), L"Value");
      res_x->register_to_ini (settings.get_file (), L"SystemSettings", L"ResX");
      res_x->load ();

      res_y->register_to_xml (FindOption (bmak_gamesettings, L"ResolutionY"), L"Value");
      res_y->register_to_ini (settings.get_file (), L"SystemSettings", L"ResY");
      res_y->load ();

      max_fps->register_to_ini (settings.get_file (), L"SystemSettings", L"MaxFPS");
      max_fps->bind_to_control (new EditBox (GetDlgItem (hDlg, IDC_MAXFPS)));
      max_fps->load ();


      use_vsync->register_to_xml (FindOption (bmak_gamesettings, L"Vsync"), L"Value");
      use_vsync->register_to_ini (settings.get_file (), L"SystemSettings", L"UseVsync");
      use_vsync->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_VSYNC)));
      use_vsync->load ();


      smooth_framerate->register_to_ini (engine.get_file (), L"Engine.Engine", L"bSmoothFrameRate");
      smooth_framerate->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_FRAMERATE_SMOOTHING)));
      smooth_framerate->load ();

      smoothed_min->register_to_ini (engine.get_file (), L"Engine.Engine", L"MinSmoothedFrameRate");
      smoothed_min->bind_to_control (new EditBox (GetDlgItem (hDlg, IDC_MIN_SMOOTHED)));
      smoothed_min->load ();

      smoothed_max->register_to_ini (engine.get_file (), L"Engine.Engine", L"MaxSmoothedFrameRate");
      smoothed_max->bind_to_control (new EditBox (GetDlgItem (hDlg, IDC_MAX_SMOOTHED)));
      smoothed_max->load ();


      blur_samples->register_to_ini (settings.get_file (), L"SystemSettings", L"MaxFilterBlurSampleCount");
      blur_samples->load ();


      anisotropy->register_to_ini (settings.get_file (), L"SystemSettings", L"MaxAnisotropy");
      anisotropy->load ();

      texture_res->register_to_xml (FindOption (bmak_gamesettings, L"Texture_Resolution"), L"Value");
      texture_res->register_to_ini (settings.get_file (), L"SystemSettings", L"TextureResolution");
      texture_res->load ();


      // A checkbox to _disable_ something's a little awkward - we have to rename this...
      Button_SetText (GetDlgItem (hDlg, IDC_HARDWARE_PHYSX), L"Disable Hardware PhysX");
      hardware_physx->register_to_ini (engine.get_file (), L"Engine.Engine", L"bDisablePhysXHardwareSupport");
      hardware_physx->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_HARDWARE_PHYSX)));
      hardware_physx->load ();

      physx_level->register_to_ini (engine.get_file (), L"Engine.Engine", L"PhysXLevel");
      physx_level->load ();

      physx_heap_size->register_to_ini (engine.get_file (), L"Engine.Engine", L"PhysXGpuHeapSize");
      physx_heap_size->load ();

      physx_mesh_cache->register_to_ini (engine.get_file (), L"Engine.Engine", L"PhysXMeshCacheSize");
      physx_mesh_cache->load ();


      enable_dx10->register_to_ini (settings.get_file (), L"SystemSettings", L"AllowD3D10");
      enable_dx10->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_DX10)));
      enable_dx10->load ();

      enable_dx11->register_to_ini (settings.get_file (), L"SystemSettings", L"AllowD3D11");
      enable_dx11->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_DX11)));
      enable_dx11->load ();

      enable_crossfire->register_to_ini (settings.get_file (), L"SystemSettings", L"bEnableCrossfire");
      enable_crossfire->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_CROSSFIRE)));
      enable_crossfire->load ();


      level_of_detail->register_to_xml (FindOption (bmak_gamesettings, L"Level_Of_Detail"), L"Value");
      level_of_detail->register_to_ini (settings.get_file (), L"SystemSettings", L"LevelOfDetail");
      level_of_detail->load ();

      //level_of_detail2.register_to_xml (FindOption (bmak_gamesettings, L"Level_Of_Detail"), L"Value");
      //level_of_detail2.register_to_ini (settings.get_file (), L"SystemSettings", L"DetailMode");
      //level_of_detail2.load ();

      shadow_quality->register_to_xml (FindOption (bmak_gamesettings, L"Shadow_Quality"), L"Value");
      shadow_quality->register_to_ini (settings.get_file (), L"SystemSettings", L"ShadowQuality");
      shadow_quality->load ();

      antialiasing->register_to_xml (FindOption (bmak_gamesettings, L"Anti-Aliasing"), L"Value");
      antialiasing->register_to_ini (settings.get_file (), L"SystemSettings", L"Antialiasing");
      antialiasing->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_ANTIALIASING)));
      antialiasing->load ();


      interactive_debris->register_to_xml (FindOption (bmak_gamesettings, L"Interactive_Paper_Debris"), L"Value");
      interactive_debris->register_to_ini (settings.get_file (), L"SystemSettings", L"bEnableInteractivePaperDebris");
      interactive_debris->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_INTERACTIVE_DEBRIS)));
      interactive_debris->load ();

      interactive_smoke->register_to_xml (FindOption (bmak_gamesettings, L"Interactive_Smoke"), L"Value");
      interactive_smoke->register_to_ini (settings.get_file (), L"SystemSettings", L"bEnableInteractiveSmoke");
      interactive_smoke->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_INTERACTIVE_SMOKE)));
      interactive_smoke->load ();

      enhanced_rain->register_to_xml (FindOption (bmak_gamesettings, L"Rain_FX"), L"Value");
      enhanced_rain->register_to_ini (settings.get_file (), L"SystemSettings", L"bEnableRainFX");
      enhanced_rain->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_ENHANCED_RAIN)));
      enhanced_rain->load ();

      enhanced_lightshafts->register_to_xml (FindOption (bmak_gamesettings, L"Volumetric_Lighting"), L"Value");
      enhanced_lightshafts->register_to_ini (settings.get_file (), L"SystemSettings", L"bEnableVolumetricLighting");
      enhanced_lightshafts->bind_to_control (new CheckBox (GetDlgItem (hDlg, IDC_ENHANCED_LIGHT_SHAFTS)));
      enhanced_lightshafts->load ();


      mip_fadein0->register_to_ini (engine.get_file (), L"Engine.Engine", L"MipFadeInSpeed0");
      mip_fadein0->load ();
      mip_fadein1->register_to_ini (engine.get_file (), L"Engine.Engine", L"MipFadeInSpeed1");
      mip_fadein1->load ();

      mip_fadeout0->register_to_ini (engine.get_file (), L"Engine.Engine", L"MipFadeOutSpeed0");
      mip_fadeout0->load ();
      mip_fadeout1->register_to_ini (engine.get_file (), L"Engine.Engine", L"MipFadeOutSpeed1");
      mip_fadeout1->load ();


      shadow_scale->register_to_ini (settings.get_file (), L"SystemSettings", L"ShadowTexelsPerPixel");
      shadow_scale->load ();



      framerate_limiting->register_to_ini (engine.get_file (), L"Engine.Engine", L"FrameRateLimitingSetting");
      framerate_limiting->load ();

      max_delta_time->register_to_ini (engine.get_file (), L"Engine.GameEngine", L"MaxDeltaTime");
      max_delta_time->load ();

      visibility_frames->register_to_ini (engine.get_file (), L"Engine.Engine", L"PrimitiveProbablyVisibleTime");
      visibility_frames->load ();


      //
      // Now, for the non-game settings -- [BMT.User]
      //
      if (first_load) { // Keep this preference across configuration restorations
        decline_backup =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"NoBackup")
          );

        decline_backup->register_to_ini (settings.get_file (), L"BMT.User", L"NoBackup");
        decline_backup->load ();
      }
      else {
        // This isn't enough, we need to actually save BmSystemSettings.ini to make sure
        //   config files retain this preference -- @FIXME: Persistent backup preference.
        decline_backup->store ();
      }

      streaming_profile->register_to_ini (engine.get_file (), L"BMT.User", L"StreamingProfileID");
      streaming_profile->load ();

      texgroup_profile->register_to_ini (settings.get_file (), L"BMT.User", L"TexGroupProfileID");
      texgroup_profile->load ();


      setup_resolution       (hDlg);
      setup_physx_properties (hDlg);
      setup_blur_samples     (hDlg);
      setup_tex_filter       (hDlg);
      setup_tex_res          (hDlg);
      setup_level_of_detail  (hDlg);
      setup_shadow_quality   (hDlg);
      setup_shadow_scale     (hDlg);

      setup_driver_tweaks (hDlg);

      setup_framerate_limiting (hDlg);
      setup_max_delta_time     (hDlg);
      setup_visibility_frames  (hDlg);

      setup_fadein  (hDlg);
      setup_fadeout (hDlg);


      std::wstring gpu_str = DXGI::GetGPUInfo ();

      HFONT font = CreateFont (11, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, MONO_FONT, L"Consolas");

      SetWindowFont (GetDlgItem (hDlg, IDC_GPUINFO), font, true);
      Edit_SetText (GetDlgItem (hDlg, IDC_GPUINFO), gpu_str.c_str ());


      xml_node <wchar_t>* display_mode =
        FindOption (bmak_gamesettings, L"Display_Mode");
      xml_attribute <wchar_t>* display_mode_val =
        FindAttrib (display_mode, L"Value");

      int window_mode = _wtoi (display_mode_val->value ());

      // @TODO: Radio button wrapper, doing it this way is just plain ridiculous!

      if (window_mode == 0) {
        mode = 0;
        handle_window_radios (hDlg, IDC_BORDER_WINDOW);
      }
      else if (window_mode == 1) {
        // We have two borderless modes, one standard and this special one (change desktop res at launch)
        if (settings.lookup_value (L"SystemSettings", L"WindowDisplayMode") == L"3") {
          mode = 1;
          handle_window_radios (hDlg, IDC_BORDERLESS_WINDOW);
        }
        else {
          mode = 3;
          handle_window_radios (hDlg, IDC_BORDERLESS_WINDOW);
        }
      }
      else {
        mode = 2;
        handle_window_radios (hDlg, IDC_FULLSCREEN);
      }


      // Visual indication as to which profile is selected...
      switch (streaming_profile->get_value ())
      {
        default:
        case 0:
          Button_SetCheck (GetDlgItem (hDlg, IDC_DEFAULT_STREAMING), TRUE);
          break;
        case 1:
          Button_SetCheck (GetDlgItem (hDlg, IDC_SLOW_STREAMING), TRUE);
          break;
        case 2:
          Button_SetCheck (GetDlgItem (hDlg, IDC_BALANCED_STREAMING), TRUE);
          break;
        case 3:
          Button_SetCheck (GetDlgItem (hDlg, IDC_FAST_STREAMING), TRUE);
          break;
      }
      //Button_Enable (GetDlgItem (hDlg, IDC_DEFAULT_STREAMING),  FALSE);
      //Button_Enable (GetDlgItem (hDlg, IDC_SLOW_STREAMING),     FALSE);
      //Button_Enable (GetDlgItem (hDlg, IDC_BALANCED_STREAMING), FALSE);
      //Button_Enable (GetDlgItem (hDlg, IDC_FAST_STREAMING),     FALSE);

      // Visual indication for TexGroup policy.
      switch (texgroup_profile->get_value ())
      {
        default:
        case 0:
          Button_SetCheck (GetDlgItem (hDlg, IDC_DEFAULT_TEXGROUPS), TRUE);
          break;
        case 1:
          Button_SetCheck (GetDlgItem (hDlg, IDC_QUALITY_TEXGROUPS), TRUE);
          break;
      }


      // Visual indication for PhysX level
      int physx_bits = 0;

      physx_bits += ((physx_level->get_value () & 0x01) == 0x01);
      physx_bits += ((physx_level->get_value () & 0x02) == 0x02);
      physx_bits += ((physx_level->get_value () & 0x04) == 0x04);

      switch (physx_bits)
      {
        // 0 or 1 is low... this whole system is convoluted and the engine's handling of
        //   PhysXLevel makes no sense, but whatever.
        default:
        case 1:
          Button_SetCheck (GetDlgItem (hDlg, IDC_LOW_PHYSX), TRUE);
          break;
        case 2:
          Button_SetCheck (GetDlgItem (hDlg, IDC_HIGH_PHYSX), TRUE);
          break;
        case 3:
          Button_SetCheck (GetDlgItem (hDlg, IDC_ULTRA_PHYSX), TRUE);
          break;
      }

      first_load = false;

      //extern INT_PTR CALLBACK ImportExport (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
      //return DialogBox (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_IMPORT_EXPORT), hDlg, ImportExport);

      return (INT_PTR)TRUE;
    }

    case WM_LBUTTONDBLCLK:
    {
      bool debug = IsWindowVisible (GetDlgItem (hDlg, IDC_DEBUG_GROUP));
      debug = !debug;

      setup_debug_utils (hDlg, debug);
    } break;

    case WM_COMMAND:
    {
      // What insanity is this?! The message pump shouldn't be working while a message box is up!
      if (messagebox_active) // Ignore button clicks while a Message Box is active
        return 0;

      if (LOWORD (wParam) == IDC_BORDERLESS_WINDOW || 
          LOWORD (wParam) == IDC_BORDER_WINDOW     ||
          LOWORD (wParam) == IDC_FULLSCREEN) {
        handle_window_radios (hDlg, LOWORD (wParam));
      }
      if (LOWORD (wParam) == IDC_MATCH_DESKTOP) {
        DEVMODE dmNow;

        memset (&dmNow, 0, sizeof (DEVMODE));
            dmNow.dmSize = sizeof (DEVMODE);

        EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dmNow);

        res_x->set_value        (dmNow.dmPelsWidth);
        res_y->set_value        (dmNow.dmPelsHeight);
        refresh_rate->set_value (dmNow.dmDisplayFrequency);

        setup_resolution (hDlg);
      }

      if (LOWORD (wParam) == IDC_NV_DRIVER_TWEAKS) {
        NVAPI::bmt_fullscreen = (mode == 2); // This is a hack based on another hack (see handle_window_radios), yuck.
        return (int)DialogBox (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_DRIVER_TWEAKS), hDlg, DriverConfigNV);
      }

      if (LOWORD (wParam) == IDC_DEFAULT_STREAMING) {
        streaming_profile->set_value (0);
        engine.import (std::wstring (streaming.default));
      }

      if (LOWORD (wParam) == IDC_SLOW_STREAMING) {
        streaming_profile->set_value (1);
        engine.import (std::wstring (streaming.slow));

        // Don't do this for any profile that uses the texture file cache,
        //   that cache has a fixed-size and PoolSize must not be allowed
        //     to exceed it.
        //BMT_OptimizeStreamingMemory ();
      }

      if (LOWORD (wParam) == IDC_BALANCED_STREAMING) {
        streaming_profile->set_value (2);
        engine.import (std::wstring (streaming.balanced));
        BMT_OptimizeStreamingMemory ();
      }

      if (LOWORD (wParam) == IDC_FAST_STREAMING) {
        streaming_profile->set_value (3);
        engine.import (std::wstring (streaming.fast));
        BMT_OptimizeStreamingMemory ();
      }

      if (LOWORD (wParam) == IDC_VSYNC) {
        use_vsync->set_value (Button_GetCheck (GetDlgItem (hDlg, IDC_VSYNC)));
      }

      if (LOWORD (wParam) == IDC_DEFAULT_TEXGROUPS) {
        settings.import (std::wstring (texgroups.default));
        texgroup_profile->set_value (0);
      }

      if (LOWORD (wParam) == IDC_QUALITY_TEXGROUPS) {
        settings.import (std::wstring (texgroups.quality));
        texgroup_profile->set_value (1);
      }

      if (LOWORD (wParam) == IDC_HARDWARE_PHYSX) {
        hardware_physx->set_value (Button_GetCheck (GetDlgItem (hDlg, IDC_HARDWARE_PHYSX)) == TRUE);

        setup_physx_properties (hDlg);
      }

      if (LOWORD (wParam) == IDC_RESOLUTION) {
        if (HIWORD (wParam) == CBN_SELCHANGE) {
          int x, y, refresh;
          get_resolution (hDlg, &x, &y, &refresh);

          res_x->set_value (x);
          res_y->set_value (y);

          // Set the refresh rate whenever the resolution changes...
          setup_resolution (hDlg);
        }
      }

      // Update refresh rate immediately
      if (LOWORD (wParam) == IDC_REFRESH_RATE) {
        if (HIWORD (wParam) == CBN_SELCHANGE) {
          int x, y, refresh;
          get_resolution (hDlg, &x, &y, &refresh);
          refresh_rate->set_value (refresh);
        }
      }

      if (LOWORD (wParam) == IDC_PHYSX_GPU) {
        if (HIWORD (wParam) == CBN_SELCHANGE)
          setup_physx_properties (hDlg);
      }

      if (LOWORD (wParam) == IDC_LOW_PHYSX)
      {
        //physx_level->set_value (1);
        physx_level->set_value (0);
      }

      if (LOWORD (wParam) == IDC_HIGH_PHYSX)
      {
        physx_level->set_value (3);
      }

      if (LOWORD (wParam) == IDC_ULTRA_PHYSX)
      {
        physx_level->set_value (7);
      }

      if (LOWORD (wParam) == IDC_PHYSX_MEMTUNE)
      {
        size_t heap;
        size_t cache;

        tune_physx_memory (hDlg, heap, cache);

        physx_heap_size->set_value  (heap);
        physx_mesh_cache->set_value (cache);
      }

      if (LOWORD (wParam) == IDC_NUKE_CONFIG)
      {
        if (MessageBox (NULL, L"WARNING: This will delete all game settings and restore you to default settings the next time you run the game.\n\n"
                              L"\tDo you really wish to continue?",
                              L"Please Confirm An Irreversable Operation",
                              MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK) {
          BMT_DeleteAllConfigFiles ();
          EndDialog (hDlg, LOWORD (wParam));
          return (INT_PTR)TRUE;
        }
      }

      if (LOWORD (wParam) == IDC_RESTORE_CONFIG)
      {
        int status =
          BMT_MessageBox (L"This will discard any changes you've made and restore configuration to an earlier state.\n\n"
                          L"\tDo you wish to continue?",
                          L"Restore Configuration From Backup?",
            MB_YESNO | MB_ICONQUESTION);

        if (status == IDYES) {

          HINSTANCE hInst = GetWindowInstance (hDlg);

          BMT_RestoreConfigFiles ();
          EndDialog (hDlg, LOWORD (wParam));

          // We're effectively going to recurse here, there's a slim
          //  possibility of a stack overflow if the user does this enough.
          return (int)DialogBox (hInst, MAKEINTRESOURCE (IDD_BATMAN), NULL, Config);
        }
      }

      if (LOWORD (wParam) == IDC_BACKUP_CONFIG)
      {
        bool allow_backup = true;

        // Confirm before overwriting an existing backup.
        if (BMT_HasBackupConfigFiles ()) {
          int status =
            BMT_MessageBox (L"A previous backup already exists.\n\n"
                            L"\tWould you like to replace it?",
                            L"Replace Existing Configuration Backup?",
              MB_YESNO | MB_ICONQUESTION);

          if (status != IDYES)
            allow_backup = false;
        }

        // Honor the user's selection
        if (allow_backup)
          BMT_CreateBackupConfig ();

        // Basically just refresh the file timestamps
        setup_debug_utils (hDlg, true);
      }

      if (LOWORD (wParam) == IDCANCEL)
      {
        NVAPI::UnloadLibrary ();
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }

      else if (LOWORD (wParam) == IDOK) {
        int x, y, refresh;
        get_resolution (hDlg, &x, &y, &refresh);

        res_x->set_value    (x);
        res_x->store        ();

        res_y->set_value    (y);
        res_y->store        ();

        refresh_rate->set_value (refresh);
        refresh_rate->store     ();

        max_fps->store      ();

        use_vsync->store    ();

        smooth_framerate->store ();
        smoothed_min->store     ();
        smoothed_max->store     ();

        hardware_physx->store   ();
        physx_level->store      ();
        physx_heap_size->store  ();
        physx_mesh_cache->store ();

        blur_samples->set_value (get_blur_samples (hDlg));
        blur_samples->store     ();

        anisotropy->set_value (get_tex_filter (hDlg));
        anisotropy->store     ();

        ////

        enable_dx10->store      ();
        enable_dx11->store      ();
        enable_crossfire->store ();

        level_of_detail->set_value  (get_level_of_detail (hDlg));
        //level_of_detail2.set_value (get_level_of_detail (hDlg));

        level_of_detail->store  ();
        //level_of_detail2.store ();

        shadow_quality->set_value (get_shadow_quality (hDlg));
        shadow_quality->store     ();

        antialiasing->store ();

        interactive_debris->store   ();
        interactive_smoke->store    ();
        enhanced_rain->store        ();
        enhanced_lightshafts->store ();

        store_fadein (hDlg);
        store_fadeout (hDlg);

        texture_res->set_value (get_tex_res (hDlg));
        texture_res->store     ();

        shadow_scale->set_value (get_shadow_scale (hDlg));
        shadow_scale->store     ();

        //framerate_limiting->set_value (get_framerate_limiting (hDlg));
        framerate_limiting->set_value (7);
        framerate_limiting->store     ();

        max_delta_time->set_value (get_max_delta_time (hDlg));
        max_delta_time->store     ();

        framerate_limiting->set_value (8);
        //visibility_frames->set_value (get_visibility_frames (hDlg));
        visibility_frames->store     ();

        //
        // @TODO: UI Control Wrapper for Radio Buttons
        //

        xml_node <wchar_t>* display_mode =
          FindOption (bmak_gamesettings, L"Display_Mode");
        xml_attribute <wchar_t>* display_mode_val =
          FindAttrib (display_mode, L"Value");

        int window_mode = 0;

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW))) {
          window_mode = 0;
          display_mode_val->value (L"0");
          settings.set_value (L"SystemSettings", L"WindowDisplayMode", L"0");
        }

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW)) == 1) {
          window_mode = 1;
          display_mode_val->value (L"1");
          settings.set_value (L"SystemSettings", L"WindowDisplayMode", L"1");
        }

        // We created a custom 4th mode, in which BMT changes the desktop resolution...
        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW)) == 2) {
          window_mode = 1;
          display_mode_val->value (L"1");
          settings.set_value (L"SystemSettings", L"WindowDisplayMode", L"3");
        }

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN))) {
          window_mode = 2;
          display_mode_val->value (L"2");
          settings.set_value (L"SystemSettings", L"WindowDisplayMode", L"2");
        }

        bool cancel = false;

        // System to make an automatic backup if INI files at save-time
        if (! BMT_HasBackupConfigFiles ()) {
          // So, no backups exist - maybe the user doesn't want backups?
          if (! decline_backup->get_value ()) {
            int status =
              BMT_MessageBox (L"No backup configuration files were detected, would you like to backup your configuration now?\n\n\tIf you press [No], you will never be prompted again.",
                              L"Create Backup Configuration Files?",
                              MB_YESNOCANCEL | MB_ICONQUESTION);

            if (status == IDCANCEL)
              return (INT_PTR)TRUE;

            if (status == IDYES) {
              BMT_CreateBackupConfig    ();
              decline_backup->set_value (false);
            }
            else {
              decline_backup->set_value (true);
            }

            decline_backup->store ();
          }
        }

        streaming_profile->store ();
        texgroup_profile->store  ();

        SaveXML ();

        settings.save (install_path);
        engine.save   (install_path);

        NVAPI::UnloadLibrary ();

        EndDialog (hDlg, TRUE);
        return (INT_PTR)TRUE;
      }
    } break;

    case WM_DESTROY:
    {
      NVAPI::UnloadLibrary ();
      //int ret = MessageBox (NULL, L"Quit Without Saving?", L"Settings Have Not Been Saved", MB_YESNOCANCEL);

      //if (ret == IDOK)
        PostQuitMessage (0);
      //else

    } break;
  }

  return (INT_PTR)FALSE;
}