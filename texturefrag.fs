/* Texture map fragment shader */

// TODO: Sampler variable
uniform sampler2D texMap;

void main()
{
	// TODO: Sample texture
	vec4 texColor = texture2D(texMap, gl_TexCoord[0].st);

	// TODO: Apply texture map using replacement
	gl_FragColor = texColor;
}