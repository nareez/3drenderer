pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
//main
function project(point)
 projected_point = {}
 projected_point.x = (fov_factor * point[x_ix]) / point[z_ix]
 projected_point.y = (fov_factor * point[y_ix]) / point[z_ix]
 return projected_point
end

function _init()
 cls(0)
end

function _update()
 cube_rotation.x += 0.005
 cube_rotation.y += 0.006
 cube_rotation.z += 0.007

	for i=1,#cube_faces do
	 mesh_face = cube_faces[i]
	 face_vertices = {}

	 face_vertices[1] = cube_vertices[mesh_face[a_ix]]
	 face_vertices[2] = cube_vertices[mesh_face[b_ix]]
	 face_vertices[3] = cube_vertices[mesh_face[c_ix]]
	 
	 projected_triagle = {}
	 
	 for j=1,3 do
	  
	  transformed_vertex = face_vertices[j]
	  transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x)
	  transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y)
	  transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z)	 
   
   transformed_vertex[z_ix] -= camera_position.z
   
   projected_point = project(transformed_vertex)
   
   projected_point.x += display_w_or_h / 2
   projected_point.y += display_w_or_h / 2
   
   projected_triagle[j] = projected_point
   	 
	 end
	 
	 add(triangles_to_render, projected_triagle)
	 
	end
end

function _draw()
 cls(0)
 for i,triangle in ipairs(triangles_to_render) do
  draw_triangle(triangle[1].x, triangle[1].y,
                triangle[2].x, triangle[2].y,
                triangle[3].x, triangle[3].y,
                col)
 end
 triangles_to_render = {}
end


-->8
//display
display_w_or_h = 128 

function draw_triangle(x0,y0,x1,y1,x2,y2,col)
 line(x0,y0,x1,y1,col)
 line(x1,y1,x2,y2,col)
 line(x2,y2,x0,y0,col)
 colo = 4
 rectfill(x0,y0,x0+2,y0+2,colo)
 rectfill(x1,y1,x1+2,y1+2,colo)
 rectfill(x2,y2,x2+2,y2+2,colo)
end


-->8
//global variables
x_ix = 1
y_ix = 2
z_ix = 3

a_ix = 1
b_ix = 2
c_ix = 3

col = 12 //color

camera_position = {}
camera_position.z = -30

fov_factor = 640

triangles_to_render = {}

cube_rotation = {}
cube_rotation.x = 0
cube_rotation.y = 0
cube_rotation.z = 0

cube_vertices = {
    { -1, -1, -1 }, // 1
    { -1,  1, -1 }, // 2
    {  1,  1, -1 }, // 3
    {  1, -1, -1 }, // 4
    {  1,  1,  1 }, // 5
    {  1, -1,  1 }, // 6
    { -1,  1,  1 }, // 7
    { -1, -1,  1 }  // 8
}

cube_faces = {
    // front
    {1,2,3},
    {1,3,4},
    // right
    {4,3,5},
    {4,5,6},
    // back
    {6,5,7},
    {6,7,8},
    // left
    {8,7,2},
    {8,2,1},
    // top
    {2,7,5},
    {2,5,3},
    // bottom
    {6,8,1},
    {6,1,4}
}
-->8
//vector
function vec3_rotate_x(v,angle)
 rotated_vector = {
  v[x_ix],
  v[y_ix] * cos(angle) - v[z_ix] * sin(angle),
  v[y_ix] * sin(angle) + v[z_ix] * cos(angle)
 }
 return rotated_vector
end

function vec3_rotate_y(v,angle)
 rotated_vector = {
  v[x_ix] * cos(angle) - v[z_ix] * sin(angle),
  v[y_ix],
  v[x_ix] * sin(angle) + v[z_ix] * cos(angle)
 }
 return rotated_vector
end

function vec3_rotate_z(v,angle)
 rotated_vector = {
  v[x_ix] * cos(angle) - v[z_ix] * sin(angle),
  v[x_ix] * sin(angle) + v[z_ix] * cos(angle),
  v[z_ix]
 }
 return rotated_vector
end
__gfx__
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00700700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
