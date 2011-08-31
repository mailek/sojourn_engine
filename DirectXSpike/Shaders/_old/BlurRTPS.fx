sampler Texture0;
float fInverseViewportWidth;
float fInverseViewportHeight;

const vector samples[9] = {
//format is [texcoordX, texcoordY, 0, sample factor]
   0.0f,   0.0f,   0.0f,    6.0f/9.0f,
   -1.0f,  0.0f,   0.0f,    0.5f/9.0f,
   1.0f,   0.0f,   0.0f,    0.5f/9.0f,
   0.0f,   1.0f,   0.0f,    0.5f/9.0f,
   0.0f,   -1.0f,  0.0f,    0.5f/9.0f,
   -1.0f,  1.0f,   0.0f,   0.25f/9.0f,
   -1.0f,  -1.0f,  0.0f,   0.25f/9.0f,
   1.0f,   1.0f,   0.0f,   0.25f/9.0f,
   1.0f,   -1.0f,  0.0f,   0.25f/9.0f
};

float4 ps_main(float2 Txr1 : TEXCOORD0) : COLOR0
{  
   float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
   for(int i=0; i<9; i++)
   {
      color += samples[i].w*tex2D(Texture0, Txr1+float2(samples[i].x*fInverseViewportWidth, samples[i].y*fInverseViewportHeight));  
   }
   
   return color;
   //return tex2D(Texture0, Txr1);
   
}