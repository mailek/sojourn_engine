uniform float fInverseViewportWidth;
uniform float fInverseViewportHeight;

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