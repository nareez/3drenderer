pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
//main
function project(point)
 projected_point = {}
 projected_point.x = (fov_factor * point.x) / (point.z)
 projected_point.y = (fov_factor * point.y) / (point.z)
 return projected_point
end

function _init()
 cls(0)
end

function _update60()
 cube_rotation.x += 0.002
 cube_rotation.y += 0.002
 cube_rotation.z += 0.002

	for i=1,#cube_faces do
	 mesh_face = cube_faces[i]
	 face_vertices = {}
    
	 face_vertices[1] = cube_vertices[mesh_face.a]
	 face_vertices[2] = cube_vertices[mesh_face.b]
	 face_vertices[3] = cube_vertices[mesh_face.c]
	 
	 projected_triagle = {}
	 
	 for j=1,3 do
	  
	  transformed_vertex = face_vertices[j]
	  transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x)
	  transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y)
	  transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z)	 
   
   transformed_vertex.z -= camera_position.z
   
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
 colo = 10
 rectfill(x0,y0,x0+1,y0+1,colo)
 rectfill(x1,y1,x1+1,y1+1,colo)
 rectfill(x2,y2,x2+1,y2+1,colo)
end


-->8
//global variables
col = 11 //color

camera_position = {}
camera_position.z = -19

fov_factor = 680

triangles_to_render = {}

cube_rotation = {}
cube_rotation.x = 0
cube_rotation.y = 0
cube_rotation.z = 0

cube_vertices = {
    { x=-1, y=-1, z=-1 }, // 1
    { x=-1, y=1,  z=-1 }, // 2
    { x=1,  y=1,  z=-1 }, // 3
    { x=1,  y=-1, z=-1 }, // 4
    { x=1,  y=1,  z=1 }, // 5
    { x=1,  y=-1, z=1 }, // 6
    { x=-1, y=1,  z=1 }, // 7
    { x=-1, y=-1, z=1 }  // 8
}

cube_faces = {
    // front
    {a=1,b=2,c=3},
    {a=1,b=3,c=4},
    // right
    {a=4,b=3,c=5},
    {a=4,b=5,c=6},
    // back
    {a=6,b=5,c=7},
    {a=6,b=7,c=8},
    // left
    {a=8,b=7,c=2},
    {a=8,b=2,c=1},
    // top
    {a=2,b=7,c=5},
    {a=2,b=5,c=3},
    // bottom
    {a=6,b=8,c=1},
    {a=6,b=1,c=4}
}
-->8
//vector
function vec3_rotate_x(v,angle)
 rotated_vector = {
  x=v.x,
  y=v.y * cos(angle) - v.z * sin(angle),
  z=v.y * sin(angle) + v.z * cos(angle)
 }
 return rotated_vector
end

function vec3_rotate_y(v,angle)
 rotated_vector = {
  x=v.x * cos(angle) - v.z * sin(angle),
  y=v.y,
  z=v.x * sin(angle) + v.z * cos(angle)
 }
 return rotated_vector
end

function vec3_rotate_z(v,angle)
 rotated_vector = {
  x=v.x * cos(angle) - v.y * sin(angle),
  y=v.x * sin(angle) + v.y * cos(angle),
  z=v.z
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
