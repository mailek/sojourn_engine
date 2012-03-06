uniform matrix matWorld;
uniform matrix matView;
uniform matrix matViewProjection;
uniform float4 drawColor;

struct VS_INPUT 
{
   float4 position : POSITION0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION0;
   float4 diffuse  : COLOR0;
   float1 fog      : FOG;
   
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));
   Output.diffuse = drawColor;   
   Output.fog = pow(1-(((Output.position.z-100)/800) * 0.20),2);
   
   return( Output );
   
}
