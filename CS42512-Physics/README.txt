Author: Brandon Shea
Assignment 6 Shootin' Fish into a Barrel
Fall 2014

Purpose:
This program shows off some trajectory physics and collision detection in the form of a game. In this game, the object is to shoot fish at each given barrel for points. You get up to 21 shots, and the starting level has 7 barrels.  Use the slider GUI to adjust your velocities and your speed. Launch your fish with the space bar.  There is also a reset button to move your fish back into position if you shoot too far. The camera is set to a default position that should be decent to shoot from. If you need to move the camera, use the 'wasd' keys. The game will end when you've shot all targets or you've exceeded 21 shots, where it will display a menu with a button to reset the game. Enjoy!!

Slider Notes:
This is assuming starting camera position.
x Velocity for adjust orientation left or right
y Velocity for how high up you shoot
z Velocity for how far you shoot

Other Notes:
Collision detection is using bounding box intersection checks

There are also keys to adjust firing settings. () -= {}

You get a higher score for a hit, if less shots have been fired.

Bugs:
Hitting space bar while in flight will apply even more velocity to the fish, flying even further out. This uses up a shot

The orientation of the fish can be wonky after a reload, but adjusting the orientation will adjust it.

The game ends after firing the 21st shot. This can be unfortunate if you score on this shot...