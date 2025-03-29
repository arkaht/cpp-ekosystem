#version 330

uniform mat4 u_world_transform, u_view_projection;
uniform float u_time;

layout( location = 0 ) in vec3 in_position;
layout( location = 1 ) in vec3 in_normal;
layout( location = 2 ) in vec2 in_uv;

out vec2 uv;
out vec3 normal;

void main() 
{
	float tile_size = 10.0f;
	vec4 origin = vec4( 0.0f, 0.0f, 0.0f, 1.0f ) * u_world_transform;

	float wind_id = ( origin.y ) * tile_size;
	vec3 wind_offset = vec3(
		cos( wind_id + u_time * 1.5f ) * 0.1f,
		sin( wind_id + u_time * 1.5f ) * 0.4f,
		cos( wind_id + u_time * 0.9f ) * 0.1f
	) * in_position.z;

	vec4 pos = vec4( in_position + wind_offset, 1.0f );
	gl_Position = pos * u_world_transform * u_view_projection;

	uv = in_uv;
	normal = in_normal;
}