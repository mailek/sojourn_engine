////////////////////////////////////////////////////
// DebugDraw.fx - Debug Drawing Techniques
//
// Description: Contains techniques which render 
//				objects simple and quickly.
// 
////////////////////////////////////////////////////

uniform matrix matWorld;
uniform matrix matView;
uniform matrix matViewProjection;
uniform float4 drawColor;

static vector AmbientLightIntensity = {0.6f, 0.6f, 0.6f, 1.0f};
static vector DiffuseLightIntensity = {1.0f, 1.0f, 1.0f, 1.0f};

uniform vector directionToLight;
uniform vector ambientMaterial;
uniform vector diffuseMaterial;

/*--------------------------
-  Vertex Shader
--------------------------*/

struct VS_LIGHTCOLOR 
{
   float4 position : POSITION0;
   float4 normal   : NORMAL0;
};

struct VS_BAKECOLOR
{
   float4 position : POSITION0;
   float4 color    : COLOR0;
};

struct VS_FIXCOLOR
{
   float4 position : POSITION0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION0;
   float4 diffuse  : COLOR0;
   float1 fog      : FOG;
   
};

VS_OUTPUT vs_light_color( VS_LIGHTCOLOR Input )
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));

// transform normal by world to account for rotation
   vector normal = vector(Input.normal.xyz, 0.0f);   
   vector worldspacenormal = mul(normal, matWorld);
   vector lightDir = vector(directionToLight.xyz, 0.0f);

   float sdiffuse = dot(normalize(lightDir), normalize(worldspacenormal));
   if(sdiffuse < 0.0f)
      sdiffuse = 0.0f;

   Output.diffuse = AmbientLightIntensity * ambientMaterial + sdiffuse * DiffuseLightIntensity * diffuseMaterial;   
   
// Fog
   Output.fog = pow(1-(((Output.position.z-100)/800) * .2),2);
   
   return( Output );
}

VS_OUTPUT vs_baked_color( VS_BAKECOLOR Input )
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));

   Output.diffuse = Input.color;   

// Fog
   Output.fog = pow(1-(((Output.position.z-100)/800) * .2),2);
   
   return( Output );
}

VS_OUTPUT vs_fixed_color( VS_FIXCOLOR Input )
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

technique LightColor
{
    Pass P0
    { 
        // set up renderstates
        Lighting = false;
        SpecularEnable = false;
        FogEnable = false;
        CullMode = CCW;
        AlphaBlendEnable = false;
        AlphaTestEnable = false;
        ZWriteEnable = true;
        ZEnable = true;
        //FillMode = solid;
        
        // pass setup 
        vertexshader = compile vs_2_0 vs_light_color();
        pixelshader = (ps);
   //     fvf = XYZ;
    }
}

technique XPLightColor
{
	Pass P0
	{
		// set up renderstates
		AlphaBlendEnable = true;
		BlendOp = ADD;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		CullMode = CCW;
		ZWriteEnable = true;
		ZEnable = true;
		
		// pass setup
		vertexshader = compile vs_2_0 vs_light_color();
        pixelshader = (ps);
	}
	
	Pass P1
	{
		// set up renderstates
		AlphaBlendEnable = true;
		BlendOp = ADD;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		CullMode = CW;
		ZWriteEnable = true;
		ZEnable = true;
		
		// pass setup
		vertexshader = compile vs_2_0 vs_light_color();
        pixelshader = (ps);
	}
}

technique ColoredNoTex
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
        vertexshader = compile vs_2_0 vs_baked_color();
        pixelshader = (ps);
   //     fvf = XYZ;
    }
}

technique FixedColor
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
        vertexshader = compile vs_2_0 vs_fixed_color();
        pixelshader = (ps);
 //       fvf = XYZ;
    }
}