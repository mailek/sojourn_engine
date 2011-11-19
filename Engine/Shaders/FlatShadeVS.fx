uniform matrix matWorld;
uniform matrix matView;
uniform matrix matViewProjection;

uniform vector directionToLight;
uniform vector ambientMaterial;
uniform vector diffuseMaterial;

static vector AmbientLightIntensity = {0.6f, 0.6f, 0.6f, 1.0f};
static vector DiffuseLightIntensity = {1.0f, 1.0f, 1.0f, 1.0};
static float  FogStrength = 0.13f;

struct VS_INPUT 
{
   float4 position : POSITION0;
   float4 normal   : NORMAL0;
   float2 tex1     : TEXCOORD0;
   float2 tex2     : TEXCOORD1;
   float2 tex3     : TEXCOORD2;
   float4 color    : COLOR0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION0;
   float2 tex1     : TEXCOORD0;
   float2 tex2     : TEXCOORD1;
   float2 tex3     : TEXCOORD2;
   float4 diffuse  : COLOR0;
   float1 fog      : FOG;
   
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.position = mul(Input.position, mul(matWorld, matViewProjection));
   Output.tex1 = Input.tex1;
   Output.tex2 = Input.tex2;
   Output.tex3 = Input.tex3;


// transform normal by world to account for rotation
   vector normal = vector(Input.normal.xyz, 0.0f);   
   vector worldspacenormal = mul(normal, matWorld);
   vector lightDir = vector(directionToLight.xyz, 0.0f);

   float sdiffuse = dot(normalize(lightDir), normalize(worldspacenormal));
   if(sdiffuse < 0.0f)
      sdiffuse = 0.0f;

   Output.diffuse = AmbientLightIntensity * ambientMaterial + sdiffuse * DiffuseLightIntensity * diffuseMaterial;   
   //Output.diffuse = AmbientLightIntensity * ambientMaterial + sdiffuse * DiffuseLightIntensity * diffuseMaterial;   

// Fog
//   Output.fog = 1.0f - ((Output.position.z-100.0f)/(800.0f-100.0f));
   Output.fog = pow(1-(((Output.position.z-100)/600) * FogStrength),2);
   
   return( Output );
   
}
