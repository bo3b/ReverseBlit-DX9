# ReverseBlit-DX9

This sample demonstrates how to use the NvAPI call NvAPI_Stereo_ReverseStereoBlitControl
to fetch the stereo bits from the backbuffer, for DX9 apps.  See 3Dmigoto for examples of 
how to use this API with DX11.  

The sample includes a small inset rectangle in the top left, which shows the end result
of the StretchRect operation, pasted back to the backbuffer for display.  If you cross-eye
view that inset, you will see it is in fact stereo.

The reason I'm posting this sample, is because the documentation is wrong and misleading,
and I have seen no working examples for DX9.  The sample is a modified version of the nvidia
sample code demonstrating how to properly Unproject for stereo HLSL.
<br><br>

The documentation in the conference slides for this call is flat out wrong.  It says to
use the CreateOffscreenPlainSurface as the destination.  This cannot work, because DX9
does not allow StretchRect copies from a RenderTarget like the BackBuffer to an
OffscreenPlainSurface.  Using Debug Layer, DX9 states that only a copy from an
OffscreenPlainSurface to another OffscreenPlainSurface will work. The table here for
StretchRect makes this clear: <br>
https://msdn.microsoft.com/en-us/library/windows/desktop/bb174471(v=vs.85).aspx

The documentation in the nvapi.h header file is correct, but misleading.  It states
that "In DX9 Dst surface has to be created as render target and StretchRect has to be used."
But, the destination cannot be a render target created with the obvious CreateRenderTarget.
If you use that render target, the call will report no error, but also not copy the
stereo backbuffer.  You wind up with a single eye texture, stretched 2x width.

The sequence that works is to CreateTexture with the D3DUSAGE_RENDERTARGET flag set, and
make it 2x wider than the backbuffer.  That is a Texture however, not a Surface.  StretchRect
will only copy to Surfaces.  The trick is to use GetSurfaceLevel to create a Surface for
the Texture, and use that as the target of the StretchRect.

<br><br>
The sample uses old libraries and old build tools.  It won't easily convert to newer tools.
This is setup for Visual Studio 2013.  And also requires the use of the June 2010 DirectX SDK.

It also uses the Frank Luna HR macro to clarify the code, and because that uses dxerr.lib, it will
apparently not compile with VS2015 forward, because the c-runtime changed. 

The nvidia sample code used the 2010 DirectX SDK, and there seemed little point in modernizing 
all this when we are targeting the DX9 environment.

<br>
As a small aside- Geez, what a mess. 
