# Working Notes
This document contains working notes throughout the completion of the project

## Mechanics
- Paddles
  + Entities
    - Player VS Computer
    - Computer VS Computer
    - Player VS Player
  + Movement
    - Capped vertical movement
- Ball
  + Spawns in center of the playfield
  + Initial velocity towards the player that lost the last round
    but first time a random direction?
  + Bouncing
    - Bounces of the top and bottom playfield region
    - Bounces of the paddle where the angle maybe depends on the contact angle
- Playfield
  + Vertical divider in the middle of the play field
- Round
  + Counter over the player regions
- Audio
  + Menu interaction SFX
  + Optionl music SFC
  + Paddle ball hit SFX
  + Score SFX
- Scoring
  + Score displayed per player
  + Increment score when ball over opponents region
- Menu
  + Main menu
    + Play (Mode choice)
    + Options
    + Quit to desktop
  + Game pause
    + Options
    + Back to main menu
    + Restart and whatnot
- Options
  + Window resolution and border/fullscreen
  + Display sync options
  + Input mapping
  + Resolution
  + Scaling
  + Sounds and volume
  + Overlay song
  + Variables: Padde position; ranges of motion; velocities
- Input
  + Keyboard input
  + No mouse input for menu and gameplay
- Help
  + Display contextual controls everywhere
- Platforms
  + Windows
  + Linux
- Publishing
  + Publish the thing somewhere through builds
  + Provide build tools like make/Cmake
- Q & A
  + Size and position everything to some virtual resolution or let things grow naturally?
    Also use min and max dimensions for everything.
  + Bouncing
    - How should the balls be deflected on paddle hits exactly?
      Should the player be able to deflect in a certain direction based on the contact point?
    - How to handle when the bottom or top of the paddle hits the ball?
      Deflect vertically but still bypass the hitting paddle?
  + How to make builds for different platforms?
  + How to test the game?

## Code
- Use a linter
- Implement testing and development build flags - Through executable arguments?

## Finalizing
- Fix the sync issue
- Implement debugging/dev switch
- Handle all Todo's
- Working and other un-necessary directories are removed
- Documentation
- Add all dependency reference in Makefile; source and headers

## Things to do for the next clean project
- cleanup resources on error and clean exits alike even though the OS 
  deallocates everything on the process stops
- use a linter everywhere (also checking indentation)
- specify a specific C standard to use before starting development
- decouple rendering (using sync or not) from the updates
- crossplatform directory path building
- show SDL and other dependency related errors through SDL window messages rather than loggin
  through standard files, which no-one will ever see + add some logs or something
- handling window operations properly in OpenGL when changed
- use preprocessor line numbers and names for errors etc.