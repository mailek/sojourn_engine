// 13 sample Horizontal Gauss Filter
uniform float fInverseViewportWidth;
sampler Texture0;

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

float4 ps_main(float2 Txr1 : TEXCOORD0) : COLOR0
{  
   float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
   //for(int i=0; i<13; i++)
   //{
   //   color += samples[i].y*tex2D(Texture0, float2(Txr1.x+samples[i].x*fInverseViewportWidth, Txr1.y));
   //}
   
   color = tex2D(Texture0, Txr1);
   return color;
}
