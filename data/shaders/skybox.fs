"""
process every pixel between vtxs, deciding color
"""

//direction to look on cubemap
varying vec3 v_world_position;

//cubemap texture
uniform samplerCube u_texture;

void main()
{
    //final color of pixel
    gl_FragColor = textureCube( u_texture, v_world_position);
}