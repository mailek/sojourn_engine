sampler Texture0;

float4 ps_main(float2 Txr1 : TEXCOORD0) : COLOR0
{   

   return tex2D(Texture0, Txr1);
   
}