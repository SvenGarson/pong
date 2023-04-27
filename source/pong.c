#include <screen_state_machine.h>
#include <screen_main_menu.h>

int main(void)
{
  screen_state_machine_run(screen_main_menu_make());

  /* Return to OS successfully */
  return 0;
}