#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* color_buffer_texture = NULL;

uint32_t* color_buffer = NULL;

int window_width = 1280;
int window_height = 1024;

bool init_window(void)
{
  if (0 != SDL_Init(SDL_INIT_EVERYTHING))
  {
    fprintf(stderr, "Can't init SDL\n");
    return false;
  }

  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);

  // window_width = display_mode.w - 200;
  // window_height = display_mode.h - 200;

  window = SDL_CreateWindow(
    "Software renderer",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    window_width, window_height,
    0);

  if (!window)
  {
    fprintf(stderr, "Can't create window");
    return false;
  }

  renderer = SDL_CreateRenderer(
    window, -1, 0);

  if (!renderer)
  {
    fprintf(stderr, "Can't create renderer");
    return false;
  }

  //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  return true;
}

void clear_color_buffer(uint32_t color)
{
  for (int y = 0; y < window_height; ++y)
  {
    for (int x = 0; x < window_width; ++x)
    {
      color_buffer[window_width * y + x] = color;
    }
  }
}

void draw_grid(void)
{
  for (int y = 0; y < window_height; y += 10)
  {
    for (int x = 0; x < window_width; x += 10)
    {
      color_buffer[window_width * y + x] = 0xFF555555;
    }
  }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
  int delta_x = (x1 - x0);
  int delta_y = (y1 - y0);

  int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

  float x_inc = delta_x / (float) side_length;
  float y_inc = delta_y / (float) side_length;

  float current_x = x0;
  float current_y = y0;

  for (int i = 0; i <= side_length; ++i)
  {
    draw_pixel(round(current_x), round(current_y), color);
    current_x += x_inc;
    current_y += y_inc;
  }
}

void draw_pixel(int x, int y, uint32_t color)
{
  if (x >= 0 && x < window_width && y >= 0 && y < window_height)
  {
    color_buffer[window_width * y + x] = color;
  }
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
  for (int i = 0; i < width; ++i)
  {
    for (int j = 0; j < height; ++j)
    {
      int pos_x = x + i;
      int pos_y = y + j;

      //color_buffer[window_width * pos_y + pos_x] = color;
      draw_pixel(pos_x, pos_y, color);
    }
  }
}

void render_color_buffer(void)
{
  SDL_UpdateTexture(
    color_buffer_texture,
    NULL,
    color_buffer,
    (int)(window_width * sizeof(uint32_t)));

  SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void destroy_window(void)
{
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
