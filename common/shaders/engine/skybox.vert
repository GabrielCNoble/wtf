
varying vec3 coords;
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	coords = gl_Vertex.xyz;
	//vec4 pos = gl_Vertex;
	//pos.w = 0.0;
	//pos = pos * gl_ModelViewProjectionMatrix;
	//pos.xyz /= pos.w;
	//vec4 c = gl_ProjectionMatrix *  gl_Vertex;
	
	//c.xyz /= c.w;
	
	//float qr = 
	//coords = normalize(gl_Vertex.xyz);
	//coords = gl_Vertex.xyz;
	//coords.y /= 1366.0/768.0;
	//coords = coords * gl_NormalMatrix;
	//coords = normalize(coords);
	//pos.yz = -pos.yz;
	//coords = pos.xyz;
	
}
