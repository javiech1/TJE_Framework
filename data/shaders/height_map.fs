
varying vec2 v_uv;

uniform sampler2D u_texture;

void main()
{
	float height = texture2D( u_texture, v_uv ).x;

	vec3 final_color = vec3(1.0);

	if (height < 0.1) {
		final_color = vec3(0.0, 0.6, 0.8);
	} else 
	if (height < 0.125) {
		final_color = vec3(0.9, 0.9, 0.5);
	} else 
	if (height < 0.2) {
		final_color = vec3(0.1, 0.8, 0.2);
	} else 
	if (height < 0.5) {
		final_color = vec3(0.6, 0.4, 0.2);
	}

	gl_FragColor = vec4(final_color, 1.0);
}
