# EE319K-Lab-10-Space-Invaders-PROJECT

Space Invaders


Ayan Basu & Adrian Jeyakumar


Our game will have an overlying mission of the user playing as a spaceship to destroy as many of the approaching invaders as possible. The slide potentiometer will be used to shift the location of the spaceship horizontally and a button will be used to launch the lasers that will take down each invader. As the game proceeds, a score will be kept, incrementing each time an invader has been shot by a laser. The game ends once all of the invaders have been taken down indicating a successful mission, or once an invader has reached the bottom of the screen taking over the user’s base indicating a loss. Another button will be incorporated to allow the user to pause and restart the game at their leisure. 

FEATURES:
Two buttons: one to shoot at enemy and one to pause/play the game

Slide pot: to move ship left and right

Multiple enemy ships will be sprites

Player’s ship is also a sprite

Moving sprites of bullets

Sounds: shooting sound

Score will be kept based on enemy ships destroyed or time it takes for enemy ship to be destroyed

Languages: English and Spanish


REQUIREMENTS MET:

Buttons: Shoot & Pause/Play

Slide Pot: Move spaceship (Left & Right)

Sprites: Player Ship, Enemy Ships, Bullets

Sound: (Shooting sound) created when player shoots (presses shoot button) ; (Death sound) created when enemy ship is destroyed (shown on OLED screen)

INTERRUPT SERVICE ROUTINES [
Edge Triggered Interrupt: Shoot / 
Periodic SysTick Interrupt: Move the ship Left & Right / 
Periodic Interrupt: Pause / Play Game (Interrupt set at highest priority)
]

Score is displayed at the end of the game

Language options (English and Spanish) are displayed at beginning of game (enter game screen)



**MAJOR DISCLAIMER: 
THE SOUND EFFECTS (FIRE SOUND EFFECT, SELECTION SOUND EFFECT WITH BUTTONS, DEATH SOUND EFFECT, etc.) WILL NOT OUTPUT UNLESS YOU HAVE A LICENSE (a.k.a. LICENSE CODE or LICENSE KEY) FOR KEIL uVision. (In class, we used the free version of Keil). If you want a license key for Keil, contact your professor or follow the link below: 
https://www.keil.com/download/license/
