////////////////////////////////////////////////////
// ScreenQuad.fx - Screen-oriented Textured Quad
//
// Description: Renders a texture-mapped quad that
//				covers the whole screen area.
// 
////////////////////////////////////////////////////

uniform matrix matWorld;
uniform matrix matView;
uniform matrix matViewProjection;
uniform float4 drawColor;


/*--------------------------
-  Vertex Shader
--------------------------*/

struct VS_PLAIN
{
   float4 position : POSITION0;
   float4 color    : COLOR0;
};

struct VS_COLORED
{
   float4 position : POSITION0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION0;
   float4 diffuse  : COLOR0;
   float1 fog      : FOG;
   
};

VS_OUTPUT vs_plain( VS_PLAIN Input )
{
   VS_OUTPUT Output;
   float4 colorout = {1.0, 1.0, 1.0, 1.0};

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));

   //Output.diffuse = Input.color;   
   Output.diffuse = colorout;   

// Fog
   Output.fog = pow(1-(((Output.position.z-100)/800) * .2),2);
   
   return( Output );
}

VS_OUTPUT vs_colored( VS_COLORED Input )
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));
   Output.diffuse = drawColor;   
   Output.fog = pow(1-(((Output.position.z-100)/800) * 0.20),2);
   
   return( Output );
   
}

/*--------------------------
-  Pixel Shader
--------------------------*/

float4 ps_main(vector col : COLOR0) : COLOR0
{   
   return col;  
}

pixelshader ps = compile ps_2_0 ps_main();

/*--------------------------
-  Techniques
--------------------------*/

technique PlainDraw
{
    Pass P0
    { 
        // set up renderstates
        //Lighting = false;
        //SpecularEnable = false;
        //FogEnable = false;
        //CullMode = none;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;
        //ZEnable = false;
        //FillMode = solid;
        
        // pass setup 
        vertexshader = compile vs_2_0 vs_plain();
        pixelshader = (ps);
   //     fvf = XYZ;
    }
}

technique ColorDraw
{
    Pass P0
    { 
        // set up renderstates
        //Lighting = false;
        //SpecularEnable = false;
        //FogEnable = false;
        //CullMode = none;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;
        //ZEnable = false;
        //FillMode = solid;
        
        // pass setup 
        vertexshader = compile vs_2_0 vs_colored();
        pixelshader = (ps);
 //       fvf = XYZ;
    }
}