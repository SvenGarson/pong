#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

/*
		- External functions
			+ Key pressed
			+ Key released

		- States
			+ None
			+ Pressed
			+ Held
			+ Released

		- Mapping the states
			+ On pressed (filter for OS interval input)
				- none -> pressed
			+ On released
				- [pressed, held] -> released
			+ On no input (not pressed and not released - State not the event)
				- [pressed] -> held
				- [released] -> none

		- Relating input context use to SDL key
			+ left  paddle up & down -> SLK..
			+ right paddle up & down -> etc.
			+ select
			+ back
			+ escape
*/

#endif