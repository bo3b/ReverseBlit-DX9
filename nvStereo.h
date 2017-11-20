//--------------------------------------------------------------------------------------
// File: nvStereo.h
// Authors: Samuel Gateau
// Email: devsupport@nvidia.com
//
// Util functions for stereo
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#ifndef NVSTEREO_H
#define NVSTEREO_H

//--------------------------------------------------------------------------------------
// D3D9
//--------------------------------------------------------------------------------------
#include "d3d9.h"

namespace nv
{
    class StereoParametersD3D9
    {
    public:
        /********************************* TYPES **********************************/

        /******************************* ATTRIBUTES *******************************/


        /******************************** METHODS *********************************/

        // Create / Destroy
        StereoParametersD3D9();
        virtual ~StereoParametersD3D9();

        // Create the graphics objects
        void createGraphics( IDirect3DDevice9* pd3dDevice );
        void destroyGraphics();
        
        // Update the stereo parameters data
        // Call this function everytime the stereo Separation or the stereo Convergence changes
        void updateStereoParamsMap( IDirect3DDevice9* pd3dDevice, float eyeSeparation, float separation, float convergence );

        IDirect3DTexture9* getStereoParamMapTexture() { return m_StereoParamsMap; }

    protected:

        /******************************* ATTRIBUTES *******************************/

        IDirect3DTexture9*              m_StereoParamsMap;

        /******************************** METHODS *********************************/
    };
}

//--------------------------------------------------------------------------------------
// D3D10
//--------------------------------------------------------------------------------------
#include "d3d10.h"

namespace nv
{
    class StereoParametersD3D10
    {
    public:
        /********************************* TYPES **********************************/

        /******************************* ATTRIBUTES *******************************/


        /******************************** METHODS *********************************/

        // Create / Destroy
        StereoParametersD3D10();
        virtual ~StereoParametersD3D10();

        // Create the graphics objects
        void createGraphics( ID3D10Device* pd3dDevice );
        void destroyGraphics();
        
        // Update the stereo parameters data
        // Call this function everytime the stereo Separation or the stereo Convergence changes
        void updateStereoParamsMap( ID3D10Device* pd3dDevice, float eyeSeparation, float separation, float convergence );

        ID3D10ShaderResourceView* getStereoParamMapSRV() { return m_StereoParamsMapSRV; }

    protected:

        /******************************* ATTRIBUTES *******************************/

        ID3D10Texture2D*                m_StereoParamsMap;
        ID3D10ShaderResourceView*       m_StereoParamsMapSRV;

        /******************************** METHODS *********************************/
    };
}

#endif

