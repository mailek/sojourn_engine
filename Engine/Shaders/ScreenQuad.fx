////////////////////////////////////////////////////
// ScreenQuad.fx - Screen-oriented Textured Quad
//
// Description: Renders a texture-mapped quad that
//				covers the whole screen area.
// 
////////////////////////////////////////////////////

uniform float fInverseViewportWidth;
uniform float fInverseViewportHeight;

Texture texture1;

sampler s0 = sampler_state
{
    Texture = (texture1);
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};


/*--------------------------
-  Vertex Shader
--------------------------*/

struct VS_INPUT 
{
   float3 Position : POSITION0; 
};

struct VS_OUTPUT 
{
   float4 Position : POSITION0;
   float2 Txr1     : TEXCOORD0;
};

// quad coords are in screen space (-1, -1, 0) to (1, 1, 0)
VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;
   Output.Position = float4(Input.Position.xy, 0, 1);
   Output.Txr1.x = 0.5f * (1.0f + Input.Position.x + fInverseViewportWidth);
   Output.Txr1.y = 0.5f * (1.0f - Input.Position.y + fInverseViewportHeight);
 
   return( Output );  
}


/*--------------------------
-  Pixel Shader
--------------------------*/

float4 ps_main(float2 Txr1 : TEXCOORD0) : COLOR0
{   
   return tex2D(s0, Txr1);  
}


/*--------------------------
-  Techniques
--------------------------*/

technique ScreenQuad
{
    Pass P0
    { 
        // set up renderstates
        Lighting = false;
        SpecularEnable = false;
        FogEnable = false;
        
        // pass setup 
        vertexshader = compile vs_2_0 vs_main();
        pixelshader = compile ps_2_0 ps_main();
        fvf = XYZ | TEX0;
    }
}