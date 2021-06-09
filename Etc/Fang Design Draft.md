# Fang Design Draft (06-08-2020)

## Abstract

Post-collapse colonies powered by blood machines that harvests blood from the prisoners for use in cyborg automata. Protagonist is an escapee prisoner that is attempting to destroy the blood machines and ultimately the colony.

## Gameplay

### Controls

- Pause
- Map
- Movement/strafing
- Primary fire
- Secondary fire
- Next/previous weapon 
- Interact
- Reload
- Jump
- Sprint
- Flip (combining downwards-movement + sprint, turns you 180˚ quickly without having to turn manually with the view controls)

### Player Stats

- Health
- Armor

### Weapons

#### Attributes
- Velocity: speed of the projectile (m/s)
- Distance: maximum distance of the projectile (m)
- Accuracy: amount of deviation from aim target (%)
- Recoil: amount that the weapon pushes you back when firing (J)
- Damage: damage dealt by the projectile/weapon 
- Reload: time that it takes to reload the weapon (ms)
- Speed: time in between firing the weapon (ms)
- Mode: semi-auto or full auto (true/false)
- Capacity: amount of shots per reload 

#### Weapons Ideas

##### Fists

Starting weapon (none), can throw left or right punches but nothing else.

- Primary: throw left punch
- Secondary: throw right punch

| Stat      | Value 
| :--       | --: 
| Velocity  | N/A 
| Distance  | N/A 
| Accuracy  | N/A 
| Damage    | Low 
| Reload    | N/A 
| Delay     | Low 
| Recoil    | N/A 
| Capacity  | N/A 
| Shotcount | 1
| Mode      | N/A

##### Pipe Pistol

A crude handgun made from an old pipe and DIY internals.

- Primary: fire
- Secondary: pistol whip

| Stat      | Value 
| :--       | --: 
| Velocity  | Mid
| Distance  | Mid
| Accuracy  | High
| Damage    | Low 
| Reload    | Low 
| Delay     | Low 
| Recoil    | Low 
| Capacity  | 12 
| Shotcount | 1
| Mode      | Semi-auto

##### Cast-Iron Carbine

A compact gun that can fire cast-iron spikes at high velocity + distance.

- Primary: fire
- Secondary: scope/zoom

| Stat      | Value 
| :--       | --: 
| Velocity  | High
| Distance  | High
| Accuracy  | Low / High
| Damage    | High 
| Reload    | Mid 
| Delay     | High 
| Recoil    | High 
| Capacity  | 1 
| Shotcount | 1
| Mode      | Semi-auto

##### Flak Cannon

Gib machine: fires flak rounds either solid or shattered.

- Primary: fire flak
- Secondary: fire solid

| Stat      | Value 
| :--       | --: 
| Velocity  | High
| Distance  | Low
| Accuracy  | Low
| Damage    | Mid 
| Reload    | Mid 
| Delay     | Mid 
| Recoil    | High 
| Capacity  | 6 
| Shotcount | 7
| Mode      | Semi-auto

##### Chaingun 

A Macgeyvered compression gun that splits off links from a chain and fires them at high speeds.

- Primary: fire
- Secondary: spin

| Stat      | Value 
| :--       | --: 
| Velocity  | High
| Distance  | Mid
| Accuracy  | Low
| Damage    | Mid 
| Reload    | High 
| Delay     | Low 
| Recoil    | Mid 
| Capacity  | 50 
| Shotcount | 1
| Mode      | Auto

##### LRAD

A high powered speaker that can blast apart enemies, as well as your own ear drums.

- Primary: fire
- Secondary: play music

| Stat      | Value 
| :--       | --: 
| Velocity  | High
| Distance  | Mid
| Accuracy  | N/A
| Damage    | High 
| Reload    | High 
| Delay     | High 
| Recoil    | High 
| Capacity  | 2 
| Shotcount | 1
| Mode      | Auto

##### Plastic Launcher

A cannon made for launching plastic explosives.

- Primary: fire
- Secondary: detonate

| Stat      | Value 
| :--       | --: 
| Velocity  | Low
| Distance  | Mid
| Accuracy  | Low
| Damage    | High 
| Reload    | Mid 
| Delay     | Low 
| Recoil    | Mid 
| Capacity  | 5 
| Shotcount | 1
| Mode      | Auto

##### Phaser

Laser rifle that shoots instantaneous beams of energy, but can also unload its whole charge at once.

- Primary: fire
- Secondary: discharge

| Stat      | Value 
| :--       | --: 
| Velocity  | N/A
| Distance  | N/A
| Accuracy  | Mid
| Damage    | Mid 
| Reload    | Low 
| Delay     | Low 
| Recoil    | Mid 
| Capacity  | 20 
| Shotcount | 1
| Mode      | Semi-auto

### Enemies

- Grifter
    - Low level mercenaries
    - Carry pistol or rifle
- Bastard Mech
    - Cyborgs created to protect the blood machines
    - Carry pistol, rifle, or flak cannon
- Boomer
    - Large enemies who carry LRADs
- Turret
    - Automated gunners that fire in quick succession
    - Equal power to pipe pistols
- Drone
    - Flying machinery armed with flak cannons
- Overlord
    - Head chiefs of the colonies
    - Armed with plastics launchers

### Pickups

- Armor (scrap metal)
- Health
- Ammo

## Rendering

### World

#### Geometry

- Raycast renderer to draw world geometry
- Tiles can have variable heights, but only one texture per tile
- Tile height in increments of the grid size only
- Tiles can have 4 side textures, a top texture, and a bottom texture
- Tiles can support transparent textures

#### Entities

- Drawn using billboard sprites
- Sprites are directional and animated
- Used for players, enemies, pickups, projectiles, and lights

#### Particles

- Used for blood splatter, bullet hits, smoke, gibs, etc.
- Can collide with geometry but not with entities

#### Decals

- Particles or projectiles hitting a surface draw into special textures that are rendered over top of the floor / walls

### Lighting

- Fog based on ray distance
- Per tile lighting
- Textures / sprites are lit using the light intensity for the tile they're in

### Fonts

- Monospace font map
- Ability to render using typewriter effect

### UI

- Basic interface controls such as buttons, sliders, and dropdowns
- Immediate-mode style interface

## Audio

- Directional audio
- Audio effects such as reverb and echo
- Possible support for text to speech

## Editing

- 2D level editor
- Set walls / entities / lights
- Can swap between gameplay / editing
- Potential for editing while in game mode ("3D")

## Debugging

### Console

- Similar to Source engine etc, bring up with '/'
- Commands built into game engine

### Input Recording

- Key to start/stop recording
- Grabs initial frame update data, then records input events until stopping
- Held in memory and replays until button pressed again
