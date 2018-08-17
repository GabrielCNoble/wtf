#version 400 compatibility  

uniform sampler2D UNIFORM_texture_sampler0;


//varying vec2 tex_coords;
//varying vec4 pos;


void main()
{
	gl_FragColor = texelFetch(UNIFORM_texture_sampler0, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0);
	//gl_FragData[0] = texelFetch(UNIFORM_texture_sampler0, ivec2(0, 0), 0);
	//gl_FragColor = vec4(20.0, 0, 0, 0);
}
