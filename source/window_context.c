/* Includes */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <batcher.h>
#include <audio_player.h>
#include <window_context.h>
#include <time.h>
#include <stdint.h>
#include <input_mapper.h>
#include <gameplay_dependencies.h>

/* Defines */
#define WINDOW_MAX_TITLE_LENGTH (64)
#define SCORE_TEXT_MAX_LENGTH (16)

/* Constants */
static const char * WINDOW_CONTEXT_TITLE = "Pong";
static const int WINDOW_CONTEXT_WIDTH = 800;
static const int WINDOW_CONTEXT_HEIGHT = 600;

/* Private state */
static SDL_Window * p_window = NULL;
static SDL_Surface * p_surface = NULL;
static SDL_GLContext * p_opengl_context = NULL;
pong_bool_te window_close_requested = PONG_FALSE;
static struct input_mapper_instance input_mapper;
struct gameplay_dependencies_batcher dependency_batcher;
struct gameplay_dependencies_audio dependency_audio;
struct gameplay_dependencies_input dependency_input;
struct gameplay_dependencies_windowing dependency_windowing;

/* Helper functions */
static void log_opengl_error(const char * p_tag)
{
  GLenum gl_error = glGetError();
  if (gl_error == GL_NO_ERROR) return;

  printf("\n\nOpenGL errors tagged - %s", p_tag ? p_tag : "N/A");
  int error_index = 1;
  while (gl_error != GL_NO_ERROR)
  {
    printf("\n\t[%-3d] %s", error_index++, gluErrorString(gl_error));
    gl_error = glGetError();
  }
}

static double time_in_seconds(void)
{
  return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

/* Input mapper instance wrappers */
static pong_bool_te input_mapper_none_wrapper(enum input_mapper_key_type custom_key_type)
{
  return input_mapper_custom_key_state_none(&input_mapper, custom_key_type);
}

static pong_bool_te input_mapper_pressed_wrapper(enum input_mapper_key_type custom_key_type)
{
  return input_mapper_custom_key_state_pressed(&input_mapper, custom_key_type);
}

static pong_bool_te input_mapper_held_wrapper(enum input_mapper_key_type custom_key_type)
{
  return input_mapper_custom_key_state_held(&input_mapper, custom_key_type);
}

static pong_bool_te input_mapper_released_wrapper(enum input_mapper_key_type custom_key_type)
{
  return input_mapper_custom_key_state_released(&input_mapper, custom_key_type);
}

/* Windowing dependency hook functions */
static void hook_close_request(void)
{
  window_close_requested = PONG_TRUE;
}

static pong_bool_te hook_window_is_fullscreen(void)
{
  return (SDL_GetWindowFlags(p_window) & SDL_WINDOW_FULLSCREEN) ? PONG_TRUE : PONG_FALSE;
}

static pong_bool_te hook_window_set_fullscreen(void)
{
  SDL_SetWindowFullscreen(p_window, SDL_WINDOW_FULLSCREEN);
  return hook_window_is_fullscreen();
}

static pong_bool_te hook_window_set_desktop_display_mode(void)
{
  /* Get the current display mode as default */
  SDL_DisplayMode desktop_display_mode;
  if (SDL_GetDesktopDisplayMode(0, &desktop_display_mode) < 0)
  {
    fprintf(
      stderr,
      "\n[Pong] Could not get desktop display mode - Error: %s",
      SDL_GetError()
    );
    return PONG_FALSE;
  }

  /* Quit fullscreen mode - This should be unnecessary ... */
  if (SDL_SetWindowFullscreen(p_window, 0) < 0)
  {
    fprintf(
      stderr,
      "\n[Pong] Could quit fullscreen mode - Error: %s",
      SDL_GetError()
    );
    return PONG_FALSE;
  }

  /* Set display that mode */
  if (SDL_SetWindowDisplayMode(p_window, &desktop_display_mode) < 0)
  {
    fprintf(
      stderr,
      "\n[Pong] Could not set to desktop display mode - Error: %s",
      SDL_GetError()
    );
    return PONG_FALSE;
  }

  /* Success */
  return PONG_TRUE;
}

static pong_bool_te hook_window_get_current_display_mode(SDL_DisplayMode * p_out_display_mode)
{
  return SDL_GetCurrentDisplayMode(0, p_out_display_mode) ? PONG_TRUE : PONG_FALSE;
}

static int hook_window_number_of_display_modes(void)
{
  return SDL_GetNumDisplayModes(0);
}

static void hook_window_disable_fullscreen(void)
{
  if (SDL_SetWindowFullscreen(p_window, 0) < 0)
  {
    fprintf(
      stderr,
      "\n[Pong] Could quit fullscreen mode - Error: %s",
      SDL_GetError()
    );
  }
}

static void hook_window_set_display_mode(const SDL_DisplayMode * p_chosen_display_mode)
{
  hook_window_disable_fullscreen();

  if (SDL_SetWindowDisplayMode(p_window, p_chosen_display_mode) < 0)
  {
    fprintf(
      stderr,
      "\n[Pong] Could not set to desktop display mode - Error: %s",
      SDL_GetError()
    );
  }

  /* Set window dimensions */
  SDL_SetWindowSize(p_window, p_chosen_display_mode->w, p_chosen_display_mode->h);
}

/* Function prototypes */
pong_bool_te window_context_initialize(window_context_initialize_tf p_callback_initialize)
{
	/* Initialize SDL2 */
	p_window = NULL;
  p_surface = NULL;
  p_opengl_context = NULL;

  /* Initialize SDL subsystems */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
  {
    fprintf(stderr, "\n[SDL] Could not initialize subsystems - Error: %s\n", SDL_GetError());
    return PONG_FALSE;
  }

  /* Specify SDL OpenGL window context attributes for window creation */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  /* Create the SDL rendering window */
  p_window = SDL_CreateWindow(
    WINDOW_CONTEXT_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_CONTEXT_WIDTH,
    WINDOW_CONTEXT_HEIGHT,
    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
  );

  if (p_window == NULL)
  {
    fprintf(stderr, "\n[SDL] Could not create window - Error: %s\n", SDL_GetError());
    return PONG_FALSE;
  }

  /* Create OpenGL context for created window */
  p_opengl_context = SDL_GL_CreateContext(p_window);
  if (p_opengl_context == NULL)
  {
    fprintf(stderr, "\n[SDL] Could not attribute OpenGL context to window - Error: %s\n", SDL_GetError());
    return PONG_FALSE;
  }

  /* Configure display sync option - TODO-GS: The call is correct, just not working under Ubuntu? */
  const int SDL_SYNC_NONE = 0;
  const int SDL_VSYNC = 1;
  const int SDL_SYNC_ADAPTIVE = -1;
  if (SDL_GL_SetSwapInterval(SDL_VSYNC) < 0)
  {
    fprintf(stderr, "\n[SDL] Selected display sync option not supported - Error: %s\n", SDL_GetError());
  }

  /* Initialize OpenGL state */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, WINDOW_CONTEXT_WIDTH, 0, WINDOW_CONTEXT_HEIGHT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, WINDOW_CONTEXT_WIDTH, WINDOW_CONTEXT_HEIGHT);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  log_opengl_error("\nInitialization");

  /* Initialize batch renderer */
  if (batcher_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the batch renderer");
    return PONG_FALSE;
  }

  /* Initialize audio player */
  if (audio_player_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the audio player");
    return PONG_FALSE;
  }

  /* Create input mapper */
  if (input_mapper_create(&input_mapper) == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not create an input mapper");
    return PONG_FALSE;
  }

	/* Set random time seed */
  srand(time(NULL));

  /* Prepare gameloop dependencies */
  /* Batcher for rendering */
  dependency_batcher.color = batcher_color;
  dependency_batcher.text = batcher_text;
  dependency_batcher.quadf = batcher_quadf;
  dependency_batcher.text_region = batcher_text_region;

  /* Audio player */
  dependency_audio.play_sound_effect = audio_player_play_sound_effect;

  /* Input */
  dependency_input.key_none = input_mapper_none_wrapper;
  dependency_input.key_pressed = input_mapper_pressed_wrapper;
  dependency_input.key_held = input_mapper_held_wrapper;
  dependency_input.key_released = input_mapper_released_wrapper;

  /* Windowing related */
  SDL_GetWindowSize(p_window, &dependency_windowing.window_width, &dependency_windowing.window_height);
  dependency_windowing.hook_close_window = hook_close_request;
  dependency_windowing.hook_window_is_fullscreen = hook_window_is_fullscreen;
  dependency_windowing.hook_window_set_fullscreen = hook_window_set_fullscreen; 
  dependency_windowing.hook_window_disable_fullscreen = hook_window_disable_fullscreen;
  dependency_windowing.hook_window_set_desktop_display_mode = hook_window_set_desktop_display_mode;
  dependency_windowing.hook_window_number_of_display_modes = hook_window_number_of_display_modes;
  dependency_windowing.hook_window_set_display_mode = hook_window_set_display_mode;

  /* Success governed by the external initialization callback */
  return p_callback_initialize(&dependency_windowing);
}

pong_bool_te window_context_run(window_context_gameplay_tick_tf p_callback_tick)
{
	/* Timing */
	double list_time_in_seconds_for_fps_counter = time_in_seconds();
  int frames_per_second = 0;
  double last_time_in_seconds = time_in_seconds();

  /* Gameloop */
  window_close_requested = PONG_FALSE;
  while(window_close_requested == PONG_FALSE)
  {
    /* Integration */
    const double new_time_in_seconds = time_in_seconds();
    const double dts = new_time_in_seconds - last_time_in_seconds;
    last_time_in_seconds = new_time_in_seconds;

    /* FPS counter */
    if (new_time_in_seconds - list_time_in_seconds_for_fps_counter >= 1.0)
    {
    	/* Update FPS counter in title */
      char fps_window_title[WINDOW_MAX_TITLE_LENGTH];
      snprintf(
        fps_window_title,
        WINDOW_MAX_TITLE_LENGTH,
        "%s - FPS: %d",
        WINDOW_CONTEXT_TITLE,
        frames_per_second
      );
      SDL_SetWindowTitle(p_window, fps_window_title);

      /* Reset counter */
      list_time_in_seconds_for_fps_counter = new_time_in_seconds;
      frames_per_second = 0;
    }
    frames_per_second++;

    /* Process input */
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        window_close_requested = PONG_TRUE;
    }

    /* Determine intermediate input state for all required keyboard keys */
    const uint8_t * p_keyboard_state = SDL_GetKeyboardState(NULL);
    input_mapper_set_intermediate_state(&input_mapper, p_keyboard_state);

    /* Dev close the window using escape */
    if (input_mapper_custom_key_state_pressed(&input_mapper, INPUT_MAPPER_KEY_TYPE_QUIT_APPLICATION))
      window_close_requested = PONG_TRUE;

    /* Tick the pong game */
		const pong_bool_te keep_gameloop_alive = p_callback_tick(
      dts,
      &dependency_input,
      &dependency_batcher,
      &dependency_audio,
      &dependency_windowing
    );
    if (!keep_gameloop_alive)
      break;

    /* Clear buffers and render accumulated batches */
    glClear(GL_COLOR_BUFFER_BIT);
    batcher_render();

    /* Check OpenGL errors */
    log_opengl_error("\nAfter rendering");

    /* Swap buffers */
    SDL_GL_SwapWindow(p_window);
  }

  /* Cleanup */
  batcher_cleanup();
  audio_player_cleanup();
  input_mapper_destroy(&input_mapper);
  SDL_DestroyWindow(p_window);
  SDL_Quit();
}
