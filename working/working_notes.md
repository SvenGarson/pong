# Working Notes
This document contains working notes throughout the completion of the project

## Mechanics
- Paddles
  + Entities
    - Player VS Computer
    - Computer VS Computer
    - [OK] Player VS Player
  + Movement
    - [OK] Capped vertical movement
- Ball
  + [OK] Spawns in center of the playfield
  + [OK] Random initian velocity in terms of horizontal and vertical momentum
  + [OK] Random vertical momentum towards the player that scored last
  + Bouncing
    - [OK] Bounces of the top and bottom playfield region
    - [OK] Bounce of the paddles
    - [  ] Bounde of the paddle based on the relative impact position -> game feel
- Playfield
  + [OK] Vertical divider in the middle of the play field
- Round
  + [OK] Counter over the player regions
- Audio
  + Play music and audio effects in the menu
  + Paddle sfc
  + Ball sfx
  + Score sfx
- Scoring
  + [OK] Score displayed per player
  + [OK] Increment score when ball over opponents region
  + [OK] Reset ball to center position when one player scored
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
  + Dev mode with variable sliders etc.
- Sound effects
  + Everywhere
- Juice
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
- Fixed aspect ratio i.e. virtual sized rendering
- Fix the sync issue
- Implement debugging/dev switch
- Handle all Todo's
- Working and other un-necessary directories are removed
- Documentation
- Cleanup Makefile, indicate all depenencies etc.

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