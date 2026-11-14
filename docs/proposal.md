# Game proposal

This is the proposal the team submitted before building the game, copied from [`game/ReadMe.txt`](../game/ReadMe.txt) with the formatting tidied up. The [README](../README.md) describes the game as it was finally built.

**Project title (game name):** Stealy Neal: The Golden Scarab

## 1. Game description

You play as a skilled thief infiltrating a prestigious museum to steal priceless artifacts. The game consists of two levels representing different times of day with distinct challenges. In Level 1 (Daytime Museum), you must navigate through the brightly lit exhibition halls filled with patrolling security guards, laser grids, and surveillance cameras to steal the legendary Golden Scarab from the Egyptian wing. Once obtained, Level 2 (Nighttime Storage Vault) begins, where you must escape through the dark underground vault. This level features motion sensors, pressure plates, and guard dogs in a dimly lit environment. You'll use a flashlight to navigate while collecting diamond fragments and ultimately escaping through the roof with the Crown Jewels. The game emphasizes stealth, timing, and careful navigation to avoid detection while maximizing your score through artifact collection.

## 2. First environment models (obstacles, collectibles, target)

1. Security guards (human models): patrol as obstacles
2. Laser grid posts/emitters: static obstacles
3. Surveillance cameras: rotating obstacles
4. Display cases and museum furniture: environmental obstacles
5. Small artifact collectibles (vases, statues, coins)
6. Keys: collectibles to unlock doors
7. Golden Scarab (main target): placed in a display case
8. Doors and archways: navigation elements

## 3. Second environment models (obstacles, collectibles, target)

1. Guard dogs (dog models): moving obstacles
2. Motion sensor devices: static obstacles
3. Pressure plates on the floor: trap obstacles
4. Storage boxes and crates: environmental obstacles
5. Diamond fragments: collectibles scattered around the vault
6. Flashlight (held by the player): tool model
7. Crown Jewels (main target): final objective on a pedestal
8. Vault doors and a ladder to the roof exit: navigation elements

## 4. First-person view

A first-person point of view will be implemented. The camera is the player's eye.

## 5. Third-person view

A third-person point of view will be implemented. The camera is behind and slightly above the player (the upper part of the player is visible to the camera).

## 6. Navigation

Navigation through the game is done using the keyboard and the mouse:

1. The arrow keys or WASD keys move the player forward, backward, left and right.
2. Moving the mouse controls the player's viewing direction (look around). The spacebar makes the player jump over small obstacles. A designated key (C or V) alternates between the first-person and third-person camera views.

## 7. Score

The score is displayed on the screen.

## 8. Animations with every user interaction

1. Collecting artifacts: the artifact rotates and scales down (shrinks) while a collection sound effect plays.
2. Collision with guards/obstacles: the player stumbles backward slightly with a camera shake effect and an alert sound plays; score deduction is applied.
3. Opening doors with keys: the door swings open with an animation and an unlocking sound effect.
4. Picking up the Golden Scarab: it levitates upward with a glowing effect and a triumphant sound.
5. Surveillance cameras rotate continuously (sweeping motion) with a beeping sound when detecting the player.
6. Guard dogs patrol with walking animations and bark when the player gets too close.
7. Collecting diamond fragments: they sparkle and disappear with a chiming sound.
8. Final escape through the roof: victory animation with the Crown Jewels spinning and a celebration sound.

## 9. Animated light sources

1. Level 1 (Daytime Museum): the main ceiling lights start as bright white/yellow (daytime) and gradually dim to orange/darker tones as time progresses, simulating evening approaching. This creates urgency for the player.
2. Level 2 (Nighttime Vault): the environment is dark with blue ambient lighting. The player's flashlight is a spotlight that rotates with the player's viewing direction (animated light source). Emergency lights flash red when alarms are triggered.
3. Surveillance cameras have rotating spotlight beams (white light cones) that sweep across the floor, creating moving light animations.
4. Motion sensors emit a pulsing red light that intensifies when the player is near.
5. The Crown Jewels pedestal has a rotating coloured spotlight (cycling through colours) illuminating the target.
