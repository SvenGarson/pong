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
  + Input mapping
  + Window
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
- Q & A
  + Bouncing
    - How should the balls be deflected on paddle hits exactly?
      Should the player be able to deflect in a certain direction based on the contact point?
    - How to handle when the bottom or top of the paddle hits the ball?
      Deflect vertically but still bypass the hitting paddle?

## Tech
- Implement testing and development builds

## Finalizing
- Implement debugging/dev switch
- Handle all Todo's