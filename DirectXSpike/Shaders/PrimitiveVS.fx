uniform matrix matWorld;
uniform matrix matView;
uniform matrix matViewProjection;

struct VS_INPUT 
{
   float4 position : POSITION0;
   float4 color    : COLOR0;
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
   float4 colorout = {1.0, 1.0, 1.0, 1.0};

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));

   //Output.diffuse = Input.color;   
   Output.diffuse = colorout;   


// Fog
//   Output.fog = 1.0f - ((Output.position.z-100.0f)/(800.0f-100.0f));
   Output.fog = pow(1-(((Output.position.z-100)/800) * .2),2);
   
   return( Output );
   
}
