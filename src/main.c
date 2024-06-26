#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "array.h"
#include "defines.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "vector.h"

#define M_PI  3.14159265358979323846

bool is_running = false;
int previous_frame_time = 0;
float fov_factor = 640;

triangle_t *triangles_to_render = NULL;
vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
mat4_t proj_matrix;

void render(void)
{
  //SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  //SDL_RenderClear(renderer);

  draw_grid();

  int num_triangles = array_length(triangles_to_render);

  for (int i = 0; i < num_triangles; ++i)
  {
    triangle_t triangle = triangles_to_render[i];

    if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
    {
      draw_filled_triangle(
        triangle.points[0].x, triangle.points[0].y,
        triangle.points[1].x, triangle.points[1].y,
        triangle.points[2].x, triangle.points[2].y,
        triangle.color);
    }

    if (render_method == RENDER_WIRE || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_WIRE_VERTEX)
    {
      draw_triangle(
        triangle.points[0].x, triangle.points[0].y,
        triangle.points[1].x, triangle.points[1].y,
        triangle.points[2].x, triangle.points[2].y,
        0xFF00FF00);
    }

    if (render_method == RENDER_WIRE_VERTEX)
    {
      draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFF0000);
      draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFF0000);
      draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFF0000);
    }
  }
  array_free(triangles_to_render);

  render_color_buffer();
  clear_color_buffer(0xFF000000);

  SDL_RenderPresent(renderer);
}

void setup(char *obj_file)
{
  render_method = RENDER_WIRE;
  cull_method = CULL_BACKFACE;

  color_buffer = malloc(sizeof(uint32_t) * window_width * window_height);

  if (!color_buffer)
  {
    fprintf(stderr, "Can't malloc color_buffer");
    is_running = false;
  }

  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    window_width,
    window_height);

  if (!color_buffer_texture)
  {
    fprintf(stderr, "Can't create color buffer texture");
    is_running = false;
  }

    float fov = M_PI / 3.0;
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

  //load_cube_mesh_data();
  load_obj_file_data(obj_file);
}

void process_input(void)
{
  SDL_Event event;
  SDL_PollEvent(&event);

  switch(event.type)
  {
    case SDL_QUIT:
      is_running = false;
      break;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE)
      {
        is_running = false;
      }
      if (event.key.keysym.sym == SDLK_1)
      {
        render_method = RENDER_WIRE;
      }
      if (event.key.keysym.sym == SDLK_2)
      {
        render_method = RENDER_WIRE_VERTEX;
      }
      if (event.key.keysym.sym == SDLK_3)
      {
        render_method = RENDER_FILL_TRIANGLE;
      }
      if (event.key.keysym.sym == SDLK_4)
      {
        render_method = RENDER_FILL_TRIANGLE_WIRE;
      }
      if (event.key.keysym.sym == SDLK_c)
      {
        cull_method = CULL_BACKFACE;
      }
      if (event.key.keysym.sym == SDLK_d)
      {
        cull_method = CULL_NONE;
      }
      break;
  }
}

vec2_t project(vec3_t point)
{
  vec2_t projected_point = {
    .x = (fov_factor * point.x) / point.z,
    .y = (fov_factor * point.y) / point.z
  };

  return projected_point;
}

void update(void)
{
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
  {
    SDL_Delay(time_to_wait);
  }

  previous_frame_time = SDL_GetTicks();

  triangles_to_render = NULL;

  mesh.rotation.x += 0.01;
  mesh.rotation.y += 0.01;
  mesh.rotation.z += 0.01;

  //mesh.scale.x += 0.002;

  //mesh.translation.x += 0.01;
  mesh.translation.z = 5.0;

  mat4_t scale_m = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
  mat4_t translation_m = mat4_make_translation(
    mesh.translation.x, mesh.translation.y, mesh.translation.z);
  mat4_t rotation_m_x = mat4_make_rotation_x(mesh.rotation.x);
  mat4_t rotation_m_y = mat4_make_rotation_y(mesh.rotation.y);
  mat4_t rotation_m_z = mat4_make_rotation_z(mesh.rotation.z);

  int num_faces = array_length(mesh.faces);

  mat4_t world_matrix = mat4_identity();
  world_matrix = mat4_mul_mat4(scale_m, world_matrix);
  world_matrix = mat4_mul_mat4(rotation_m_x, world_matrix);
  world_matrix = mat4_mul_mat4(rotation_m_y, world_matrix);
  world_matrix = mat4_mul_mat4(rotation_m_z, world_matrix);
  world_matrix = mat4_mul_mat4(translation_m, world_matrix);

  for (int i = 0; i < num_faces; ++i)
  {
    face_t mesh_face = mesh.faces[i];

    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    vec4_t transformed_vertices[3];

    for (int j = 0; j < 3; ++j)
    {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
      transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
      transformed_vertices[j] = transformed_vertex;
    }

    vec3_t vec_a = vec3_from_vec4(transformed_vertices[0]);
    vec3_t vec_b = vec3_from_vec4(transformed_vertices[1]);
    vec3_t vec_c = vec3_from_vec4(transformed_vertices[2]);

    vec3_t vector_ab = vec3_sub(vec_b, vec_a);
    vec3_t vector_ac = vec3_sub(vec_c, vec_a);

    vec3_normalize(&vector_ab);
    vec3_normalize(&vector_ac);

    vec3_t normal = vec3_cross(vector_ab, vector_ac);//left handed coord system
    vec3_normalize(&normal);

    vec3_t camera_ray = vec3_sub(camera_position, vec_a);

    float dot_normal_camera = vec3_dot(normal, camera_ray);

    if (cull_method == CULL_BACKFACE)
    {

      if (dot_normal_camera < 0) continue; //simple back face culling
    }

    vec4_t projected_points[3];

    for (int j = 0; j < 3; ++j)
    {
      //projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));

      projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);
      projected_points[j].y *= -1;

      projected_points[j].x *= (window_width / 2.0);
      projected_points[j].y *= (window_height / 2.0);

      projected_points[j].x += (window_width / 2.0);
      projected_points[j].y += (window_height / 2.0);

    }

    float average_depth =
      (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

    float light_intensity_factor = -vec3_dot(normal, light.direction);

    uint32_t triangle_color = mesh_face.color;
    triangle_color = light_apply_intensity(triangle_color, light_intensity_factor);

    triangle_t projected_triangle = {
      .points = {
        { projected_points[0].x, projected_points[0].y },
        { projected_points[1].x, projected_points[1].y },
        { projected_points[2].x, projected_points[2].y }
      },
      .color = triangle_color,
      .average_depth = average_depth
    };

    //simple bubble sort.
    //@todo change if too slow
    int num_triangles = array_length(triangles_to_render);

    for (int i = 0; i < num_triangles; ++i)
    {
      for (int j = i; j < num_triangles; ++j)
      {
        if (triangles_to_render[i].average_depth < triangles_to_render[j].average_depth)
        {
          triangle_t temp = triangles_to_render[i];
          triangles_to_render[i] = triangles_to_render[j];
          triangles_to_render[j] = temp;
        }
      }
    }
    array_push(triangles_to_render, projected_triangle);
  }
}

void free_resources(void)
{
  array_free(mesh.faces);
  array_free(mesh.vertices);

  free(color_buffer);
}

int main(int argc, char *argv[])
{

  char *obj_file = "./assets/cube.obj";

  if (argc > 1) {
    obj_file = argv[1];
  }

  is_running = init_window();

  setup(obj_file);

  while(is_running)
  {
    process_input();
    update();
    render();
  }

  destroy_window();
  free_resources();

  return 0;
}
