#include <screen_main_menu.h>
#include <screen_pong.h>

int main(void)
{
  /* Initialize the initial screen */
  struct screen active_screen = screen_main_menu_make();
  active_screen.p_initialize();  
  
  while(1)
  {
    /* Integrate and determine whether another screen is requested */
    const struct screen requested_screen = active_screen.p_integrate(0.016);

    /* Render the active screen */
    active_screen.p_render();

    /* Change screens if necessary */
    if (active_screen.type != requested_screen.type)
    {
      /* Cleanup the active screen */
      active_screen.p_cleanup();
      
      /* Switch to the new screen type and initialize before usage */
      active_screen = requested_screen;
      active_screen.p_initialize();
    }
  }

  /* Return to OS successfully */
  return 0;
}