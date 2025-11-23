varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform vec3 u_camera_pos;
uniform float u_time;

void main()
{
    // Normalize the normal vector
    vec3 N = normalize(v_normal);

    // Simple directional light from above-right
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float NdotL = max(dot(N, lightDir), 0.0);

    // Ambient light to prevent completely dark areas
    float ambient = 0.4;
    float diffuse = 0.6;
    float lighting = ambient + diffuse * NdotL;

    // Calculate view direction for edge highlighting
    vec3 viewDir = normalize(u_camera_pos - v_world_position);

    // Fresnel effect for edge highlighting (rim lighting)
    float fresnel = 1.0 - max(dot(viewDir, N), 0.0);
    fresnel = pow(fresnel, 2.0); // Make the effect more subtle

    // Base color with lighting
    vec3 baseColor = u_color.rgb * lighting;

    // Add edge highlight - makes platform edges pop
    vec3 edgeColor = vec3(1.0, 1.0, 1.0); // White edge highlight
    baseColor = mix(baseColor, edgeColor, fresnel * 0.3);

    // Add subtle vertical gradient for height perception
    float heightGradient = (v_world_position.y + 10.0) / 20.0; // Normalize to 0-1 range
    heightGradient = clamp(heightGradient, 0.0, 1.0);
    baseColor *= (0.85 + heightGradient * 0.15); // Subtle brightness variation

    // Add very subtle pulsing to catch attention (optional)
    float pulse = sin(u_time * 2.0) * 0.02 + 1.0;
    baseColor *= pulse;

    // Output final color
    gl_FragColor = vec4(baseColor, u_color.a);
}