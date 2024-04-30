#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "defines.h"
#include "array.h"
#include "display.h"
#include "matrix.h"
#include "mesh.h"
#include "vector.h"

bool is_running = false;
int previous_frame_time = 0;
float fov_factor = 640;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };

unsigned char fireRGB[111] = {
  0x07,0x07,0x07,0x1F,0x07,0x07,0x2F,0x0F,0x07,0x47,0x0F,0x07,0x57,0x17,0x07,0x67,
  0x1F,0x07,0x77,0x1F,0x07,0x8F,0x27,0x07,0x9F,0x2F,0x07,0xAF,0x3F,0x07,0xBF,0x47,
  0x07,0xC7,0x47,0x07,0xDF,0x4F,0x07,0xDF,0x57,0x07,0xDF,0x57,0x07,0xD7,0x5F,0x07,
  0xD7,0x5F,0x07,0xD7,0x67,0x0F,0xCF,0x6F,0x0F,0xCF,0x77,0x0F,0xCF,0x7F,0x0F,0xCF,
  0x87,0x17,0xC7,0x87,0x17,0xC7,0x8F,0x17,0xC7,0x97,0x1F,0xBF,0x9F,0x1F,0xBF,0x9F,
  0x1F,0xBF,0xA7,0x27,0xBF,0xA7,0x27,0xBF,0xAF,0x2F,0xB7,0xAF,0x2F,0xB7,0xB7,0x2F,
  0xB7,0xB7,0x37,0xCF,0xCF,0x6F,0xDF,0xDF,0x9F,0xEF,0xEF,0xC7,0xFF,0xFF,0xFF
};

#define length ((sizeof(fireRGB) / sizeof(fireRGB[0])) / 3)

vec3_t palette[length];

uint32_t* pixel_buffer = NULL;

void render(void)
{
  //SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  //SDL_RenderClear(renderer);

  for (int x = 0; x < window_width; ++x)
  {
    for (int y = 0; y < window_height; ++y)
    {
      uint32_t index = pixel_buffer[window_width * y + x];
      vec3_t color = palette[index];

      color_buffer[(window_width * y + x)] = (uint8_t)255 << 24 | (uint8_t)color.x << 16 | (uint8_t)color.y << 8 | (uint8_t)color.z;
    }
  }

  render_color_buffer();
  clear_color_buffer(0xFF00FF00);

  SDL_RenderPresent(renderer);
}

void setup(char *obj_file)
{
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

  pixel_buffer = malloc(sizeof(uint32_t) * window_width * window_height);

  if (!pixel_buffer)
  {
    fprintf(stderr, "Can't malloc pixel_buffer");
    is_running = false;
  }

  for (int y = 0; y < 37; ++y)//36 colors in palette
  {
    //xyz = rgb
    vec3_t color = {
      .x = fireRGB[y * 3 + 0],
      .y = fireRGB[y * 3 + 1],
      .z = fireRGB[y * 3 + 2]
    };

    palette[y] = color;
  }

  for (int i = 0; i < window_width * window_height; i++)
  {
    pixel_buffer[i] = 0;//black
  }
  for (int i = 0; i < window_width; i++)
  {
    for (int y = 0; y < 10; y++)
    {
      pixel_buffer[(window_height - 1) * window_width + i] = 36;//36 = white
    }
  }

  srand(time(NULL));
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
  }
}

void fire(uint32_t from)
{
  uint32_t pixel = pixel_buffer[from];

  if (pixel == 0)
  {
    pixel_buffer[from - window_width] = pixel_buffer[from];
  }
  else
  {
    uint32_t randIx = round((rand() % RAND_MAX) / (double)RAND_MAX * 3.0);
    uint32_t to = from - randIx + 1;
    pixel_buffer[to - window_width] = pixel - (randIx & 1);
  }
}

void update(void)
{
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
  {
    SDL_Delay(time_to_wait);
  }

  previous_frame_time = SDL_GetTicks();

  for (int x = 0; x < window_width; x++)
  {
    for (int y = 1; y < window_height; y++)
    {
      fire(y * window_width + x);
    }
  }
}

void free_resources(void)
{
  array_free(mesh.faces);
  array_free(mesh.vertices);

  free(pixel_buffer);
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
