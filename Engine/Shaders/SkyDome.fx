////////////////////////////////////////////////////
// Skydome.fx - Skydome Shader
//
// Description: Renders skydome without lighting.
// 
////////////////////////////////////////////////////

uniform matrix matWorld;
uniform matrix matViewProjection;

Texture diffuseTex;

sampler s0 = sampler_state
{
    Texture = (diffuseTex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


/*--------------------------
-  Vertex Shader
--------------------------*/

struct VS_INPUT {
    vector position		: POSITION;
    float2 uv1			: TEXCOORD;
};

struct VS_OUTPUT {
    vector position		: POSITION;
    float2 uv1			: TEXCOORD;
    vector color		: COLOR;
};

VS_OUTPUT vs_main(VS_INPUT Input)
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));
   Output.uv1 = Input.uv1;
   Output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
   
   return Output;
}

vertexshader vs = compile vs_2_0 vs_main();


/*--------------------------
-  Pixel Shader
--------------------------*/

/*--------------------------
-  Techniques
--------------------------*/

technique FlatTexture
{
    Pass P0
    { 
        // set up renderstates
        Lighting = false;
        SpecularEnable = false;
        FogEnable = false;
        CullMode = none;
        
        // pass setup 
        vertexshader = (vs);
        pixelshader = null;
        fvf = XYZ | TEX0;
    }
}