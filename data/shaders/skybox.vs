// process every vertex of the 3d model

//position of the 3d vertex of the cubemap
attribute vec3 a_vertex;

//matrx position
uniform mat4 u_viewprojection;

//camera position
uniform vec3 u_camera_pos;

//data passed to fragment shader
varying vec3 v_world_position;

void main()
{
    //calculate world position
    //moves cube so it is always centered on camera
    vec3 world_position = a_vertex + u_camera_pos;

    //original direction of vtx to search cubemap
    v_world_position = a_vertex;

    //final position of vertex on screen
    //convert pos 3D to 2D
    gl_Position = u_viewprojection * vec4(world_position, 1.0);

    //set w component to far plane distance 
    //TODO: review why this is needed (AI recommended)
    gl_Position = gl_Position.xyww;
}