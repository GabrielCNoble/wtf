uniform sampler2D UNIFORM_texture_sampler0;

varying vec2 tex_coord;

void main()
{
	float A = 0.15;
    float B = 0.5;
    float C = 0.1;
   	float D = 0.2;
   	float E = 0.02;
   	float F = 0.3;
   	
	vec4 texel = texture2D(UNIFORM_texture_sampler0, tex_coord);
	gl_FragColor = ((texel * (A * texel + C * B) + D * E) / (texel * (A * texel + B) + D * F)) - E / F;
}
