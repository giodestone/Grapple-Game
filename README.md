# Grapple Game
![GIF of player fling while using the Grapple Hook](TODO)
Game made in Unreal Engine 4 C++ with a grappling hook which can grapple, and whip objects and turrets; as well as a wall climbing system. Made as coursework for a module in university.


## Running
![Image of scene](TODO)
Try to get to the end of the level, avoiding the turrets (try to knock them over). You can grapple to surfaces marked in blue, and whip any items that are not dark gray or water. Climb any walls which are a cobble-like material.

### Controls
* W, A, S, D - Move around.
* Mouse - Look around.
* Left Mouse Button - Fire grapple.
* Space - Climb.


## Key Features
The key features are outlined below. Functionality is implemented in C++ using Actor Components.

### Grapple Hook
![Player in mid air](TODO)
The grapple hook moves the player to the position by first checking if the path is valid, making a CableComponent visible, and then moving the player towards the position at a set speed by using interpolation. The movement is interrupted if the player hits anything or if they cancel the grapple. The player keeps the momentum they had. Actors tagged 'NoGrapple' or 'Whipable' cannot be grappled to.

![GIF of player whipping item, sending it flying](TODO)
The whipping is implementing by applying a force on the point hit, as determined by TraceLine. A Cable Component is briefly displayed. The actor hit must be tagged 'Whipable'. 

### Wall Climbing
![Climbable wall](TODO)
A wall climbing mechanic allows the player to scale walls. It works by using two Arrows - one in the middle of the player, and one at foot level. The player is only propelled upwards as long as the foot arrow is touching within a certain distance. Both arrows must be touching a climbable surface to begin climbing. The collision check is performed in the 'Ledge' collision channel - any surfaces that are designated climbable must be designated to block collisions in the aforementioned channel.

### Turret
![GIF of turret firing](TODO)
The turret fires at the player given that the player is in the cone of fire and is in direct sight. The calculation is done by line tracing between the turret and player, then checking if the angle between the forward of the turret and player is within the cone equation. The turret can be disabled by knocking it over.

### Health
![GUI](TODO)
The health system is implemented using Unreal OnHit() functions. If the health reaches zero, the player is respawned at the Player Start. The water does not deal any damage to the player, nor is there fall damage.
