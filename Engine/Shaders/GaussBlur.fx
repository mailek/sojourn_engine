uniform float fInvViewportWidth;
uniform float fInvViewportHeight;

texture tex0;

sampler smp0 = sampler_state
{
    Texture = (tex0);
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
};

const float2 samples[13] = {
//format is [texcoordOffset, sample factor]
   6.0f,   0.002216f,
   5.0f,   0.008764f,
   4.0f,   0.026995f,
   3.0f,   0.064759f,
   2.0f,   0.120985f,
   1.0f,   0.176033f,
   0.0f,   0.242995f,
  -1.0f,   0.176033f,
  -2.0f,   0.120985f,
  -3.0f,   0.064759f,
  -4.0f,   0.026995f,
  -5.0f,   0.008764f,
  -6.0f,   0.002216f
};

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
   Output.Txr1.x = 0.5f * (1.0f + Input.Position.x + fInvViewportWidth);
   Output.Txr1.y = 0.5f * (1.0f - Input.Position.y + fInvViewportHeight);
  
   return( Output );
}

// 13 sample Horizontal Gauss Filter
float4 ps_horz(float2 Txr1 : TEXCOORD0) : COLOR0
{  
   float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
   for(int i=0; i<13; i++)
   {
      color += samples[i].y*tex2D(smp0, float2(Txr1.x+samples[i].x*fInvViewportWidth, Txr1.y));
   }
   
   return color;
}

// 13 sample Vertical Gauss Filter
float4 ps_vert(float2 Txr1 : TEXCOORD0) : COLOR0
{  
   float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
   for(int i=0; i<13; i++)
   {
      color += samples[i].y*tex2D(smp0, float2(Txr1.x, samples[i].x*fInvViewportHeight+Txr1.y));   
   }
   
   return color;
}

vertexshader vs = compile vs_2_0 vs_main();

technique GaussBlur
{
    Pass P0
    { 
        // set up renderstates
        Lighting = false;
        SpecularEnable = false;
        FogEnable = false;
        CullMode = none;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
        ZEnable = false;
        FillMode = solid;
        
        // pass setup 
        vertexshader = (vs);
//        pixelshader = NULL;
        pixelshader = compile ps_2_0 ps_horz();
        fvf = XYZ;
    }

    Pass P1
    { 
        // set up renderstates
        Lighting = false;
        SpecularEnable = false;
        FogEnable = false;
        CullMode = none;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
        ZEnable = false;
        FillMode = solid;
        
        // pass setup 
        vertexshader = (vs);
        pixelshader = compile ps_2_0 ps_vert();
//        pixelshader = NULL;
        fvf = XYZ;
    }
}
