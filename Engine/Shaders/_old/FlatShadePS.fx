sampler Texture0;
sampler Texture1;
sampler Texture2;

static float4 clear = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 ps_main(vector col : COLOR0, float2 Txr1 : TEXCOORD0, float2 Txr2 : TEXCOORD1, float2 Txr3 : TEXCOORD2) : COLOR0
{  
   float4 val = tex2D(Texture0, Txr1) * col;

//   if(any(val.rgb))
  //     val = clear;

   return val;
   
}

