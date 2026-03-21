Wrath and Ruin
Lead Artist Production Brief v1
1. Core Art Direction

Wrath and Ruin must look like:

a tactical top-down hard-sci-fi RPG
a human frontier survival world
a world of scarcity, damage, patchwork repair, industry, dust, pressure systems, cargo, machinery, terminals, docking hardware, and lived-in interiors
a game where terrain, hazards, NPCs, interactables, and state changes are readable immediately
a game where the MUD soul survives through strong environmental storytelling, object readability, inspectability, signage, logs, terminals, and social identity

Wrath and Ruin must not look like:

glossy space opera
alien fantasy
neon cyberpunk excess
sterile white sci-fi
cartoon comedy
painterly noise that hides readability
realistic 3D-style over-rendered sprites
generic “survival sandbox junk pile” art with no faction or system clarity

Top-down readability and strong silhouettes come first. Grounded materials, readable terrain state, and clear hazards/interactables are explicitly part of the project’s visual direction.

2. Non-Negotiable Visual Rules
2.1 Camera and perspective
View style: top-down tactical, not isometric diamond
Angle: near-overhead with slight front-face cheat
Walls, machinery, props, and characters may use a small readability cheat, but the world must still feel top-down
Do not draw large side-on facades that break navigation readability
2.2 Material language

Primary materials:

worn painted metal
bare steel
oxidized metal
polymer panels
reinforced glass
hazard paint
rubberized flooring
grating
cables and conduit
dust-coated concrete / ferrocrete
industrial sealants
thermal insulation wraps
plated hull sections
2.3 Color language

Base palette:

dust browns
muted ochres
greys
blue-greys
olive greys
dark steel
dirty whites
charcoal
rust accents

Accent palette only where useful:

hazard yellow
amber
emergency red
toxic green
cold medical teal
navigation cyan
power blue
faction identifiers in controlled, muted colors

Rule:

70% neutral industrial base
20% controlled functional accents
10% alert or faction accents
2.4 Readability before detail
Every sprite must read at 100% gameplay scale
Do not rely on tiny greebles
Do not texture every surface equally
Use detail to reinforce role, wear, and material, not to fill space
Characters, hazards, lootables, terminals, and objective objects must be identifiable in under one second
2.5 Line and rendering style
No thick comic outlines
Use selective dark edge separation
Internal shading must be clean and restrained
Avoid muddy midtones
Highlights should describe material, not make everything shiny
3. Technical Standard

These are the exact production standards.

3.1 Native gameplay unit
1 world tile = 48 x 48 px export size

This becomes the foundation for terrain, floors, footprints, and prop placement.

3.2 Source vs export
Source art may be created at 2x working scale
Final in-game export must be delivered at native size
Example: a 48 x 48 tile may be painted at 96 x 96, but final delivery must include the 48 x 48 PNG
3.3 File formats

Delivery required for every approved asset:

.png export with transparency
layered source: .psd or .aseprite
contact sheet / preview sheet
naming exactly to convention
3.4 Atlas rules
Atlas max size: 2048 x 2048
Preferred atlas subset size: 1024 x 1024
Padding between sprites: 4 px
Outer transparent padding: 4 px
No sprite may touch another sprite in atlas export
3.5 Pivot/origin rules
Floor and terrain: tile center
Characters: center bottom
Walls/doors: center bottom of occupied tile
Props: visual footprint center
Ships/interior modules: defined per prefab, but always documented
3.6 Shadows
No baked giant drop shadows on world sprites
Ambient base shadow can be subtle and tight
Large cast shadows should be separate assets if needed
3.7 State consistency

If an object can change state in gameplay, the art pack must provide the state set:

idle
highlighted
active
inactive
locked
unlocked
open
closed
damaged
repaired
objective-active
exhausted / used if applicable
4. Exact Asset Specifications
4A. Modular Human Character System

This is the highest-priority asset family.

Global character standard
Frame size per body-part export: 64 x 96 px
Source size: 128 x 192 px
Pivot: x center / y bottom
Character footprint: 1 tile
Directions: 8 directions
Delivery mode: layered modular parts aligned to one universal body rig
Required character sheets
1. Base body sheet

Export:

atlas size: 2048 x 2048
cell size: 64 x 96

Contents:

male base body
female base body
androgynous base body
slim build
average build
heavy build
light skin
medium skin
dark skin
scarred variants
prosthetic limb variants
undersuit base variants
2. Head sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

neutral male heads
neutral female heads
neutral androgynous heads
older heads
scarred heads
cybernetic eye / facial implant heads
death / downed face variants
hurt face variants
3. Hair and facial hair sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

shaved
buzz cut
crew cut
short messy
cropped curls
undercut
braid
ponytail
bun
long tied back
beard short
beard long
moustache
stubble
4. Helmet and face-gear sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

soft cap
headset
work helmet
security helmet
combat helmet
pilot helmet
hazard hood
respirator
rebreather
goggles
visor up
visor down
face shield
officer cap
5. Torso / chest sheet

Export:

atlas size: 2048 x 2048
cell size: 64 x 96

Contents:

undersuit torso
dockworker torso
mechanic torso
medic torso
security vest
scavenger jacket
raider chest rig
hazard suit torso
pilot torso
relay tech torso
civilian coat
armored coat
command jacket
6. Arms / gloves sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

bare arm
long sleeve
armored arm
medical sleeve
hazard sleeve
fingerless gloves
work gloves
tactical gloves
insulated gloves
7. Legs / boots sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

utility pants
fatigues
armored pants
civilian pants
hazard legs
pilot legs
work boots
armored boots
soft shoes
knee pad variants
8. Back gear sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

no pack
satchel
backpack
field pack
oxygen tank
med pack
tool rig
battery pack
comms pack
cargo sling
9. Weapon hold sheet

Export:

atlas size: 1024 x 1024
cell size: 64 x 96

Contents:

empty hands
pistol hold
SMG hold
shotgun hold
rifle hold
baton hold
knife hold
tool hold
scanner hold
datapad hold
Character animation set

Each animation uses:

64 x 96 px per frame
8 directions
Mandatory for current slice
idle: 4 frames
walk: 8 frames
interact/use terminal: 5 frames
inspect: 4 frames
hit react: 3 frames
death/downed: 6 frames
Mandatory by alpha
run: 8 frames
aim ready: 2 frames
fire single: 3 frames
reload: 6 frames
melee strike: 5 frames
loot pickup: 4 frames
board/disembark: 5 frames
carry/haul light: 6 frames
4B. Human NPC Variants

These must be built from the modular system, not as separate one-off characters unless the NPC is story-critical.

Standard
same rig as player
same 64 x 96 frame
same animation set
faction identity comes from gear, palette, markings, posture
Required alpha NPC sets
dockworker
mechanic
medic
security
pilot
relay technician
scavenger trader
wounded survivor
settler / worker
admin / operator
mission contact
raider
scavenger gunman
looter
armored enforcer
hazard-zone marauder
frontier ambusher
Required faction-marker sheet

Export:

1024 x 1024
cell size: 64 x 96

Contents:

shoulder insignia
chest insignia
helmet stripe variants
armband colors
backpack marker patches
rank markings
cargo / service markings
frontier relay markings
shuttle crew markings
4C. Drones, Turrets, and Machine Actors
Small drone standard
export size per frame: 48 x 48 px
source size: 96 x 96 px
directions: 8
Medium drone / cargo bot standard
export size per frame: 64 x 64 px
source size: 128 x 128 px
directions: 8
Turret standard
export size per frame: 64 x 64 px
source size: 128 x 128 px
directions: 8 rotational facings
Drone/turret sheets
1. Service drone

Contents:

neutral
damaged
inactive
repair-active
scan-active
2. Security drone

Contents:

patrol
alert
firing
damaged
disabled
3. Cargo bot

Contents:

idle
loaded
unloaded
damaged
disabled
4. Wall turret

Contents:

folded
deployed
aiming
firing
offline
destroyed

Animations:

idle: 4
move/hover: 6
attack: 3
spark damage: 4
disable: 4
4D. Exterior Terrain Tilesheets

All terrain uses the same base standard.

Terrain tile standard
tile size: 48 x 48 px
source size: 96 x 96 px
atlas size per terrain family: 1024 x 1024
Every terrain family must include
1 main fill tile
4 alternate fill tiles
N edge
S edge
E edge
W edge
outer corner NW
outer corner NE
outer corner SW
outer corner SE
inner corner NW
inner corner NE
inner corner SW
inner corner SE
diagonal A
diagonal B
thin transition strip A
thin transition strip B
isolated patch
damaged version
shadowed version
Required terrain families for alpha
1. Dust plain sheet
2. Compacted dirt sheet
3. Rocky ground sheet
4. Gravel / scree sheet
5. Dead scrub sheet
6. Ferrocrete apron sheet
7. Landing pad plating sheet
8. Hazard spill ground sheet
Terrain overlay sheet

Export:

1024 x 1024
tile size: 48 x 48

Contents:

tire marks
footprints dust
cracked ground
scattered pebbles
small rock clusters
medium rocks
cable spill
scrap scatter
drainage stain
oil stain
blood small
blood medium
warning paint remnants
4E. Interior Floor and Wall Sheets
Industrial / facility interior
Floor tile standard
48 x 48 px
Wall/door standard
48 x 96 px
anchored bottom center
Facility floor sheet

Atlas:

1024 x 1024

Contents:

clean metal floor
worn metal floor
grated floor
maintenance floor
hazard-striped floor
med floor
command floor
powerless dark floor
blood-stained floor
fluid-stained floor
burn-marked floor
patched floor
Facility wall sheet

Atlas:

2048 x 2048

Contents:

straight wall horizontal
straight wall vertical
outer corner x4
inner corner x4
T-junction x4
cross junction
half wall horizontal
half wall vertical
reinforced wall
broken wall
window wall
vent wall
pipe wall
cable wall
support column
bulkhead segment
Door and hatch sheet

Atlas:

1024 x 1024
cell size: 48 x 96

Contents:

standard door closed
standard door open
standard door locked
standard door damaged
blast door closed
blast door open
blast door locked
blast door destroyed
airlock closed
airlock open
hatch closed
hatch open
maintenance crawl hatch
4F. Ship Interior Sheet

This must feel tighter, more engineered, and more compact than planetary interiors.

Standard
floor: 48 x 48
wall/hatch: 48 x 96
consoles/chairs: 96 x 96
larger machinery: 96 x 144
Ship interior floor sheet

Contents:

deck plating
cockpit floor
engineering floor
cargo hold floor
bunk floor
airlock floor
emergency-lit floor
damaged floor
patched hull floor
Ship bulkhead sheet

Contents:

bulkhead straight
bulkhead corner
hatch frame
hatch open
hatch closed
ladder hatch
viewport segment
damaged bulkhead
sealed compartment wall
emergency patch panel
hull breach tile
Ship interior prop sheet

Contents:

pilot seat
co-pilot seat
helm console
nav console
engine access panel
power panel
storage locker
cargo net
tie-down
spare parts bin
emergency suit rack
bunk
med kit shelf
4G. Exterior Building Shell Sheet
Standard
wall piece: 48 x 96
roof piece: 48 x 48
larger facade chunks: 96 x 96
tower / mast pieces: 96 x 144
Required contents
exterior wall straight
exterior wall corner
exterior inner corner
damaged exterior wall
reinforced exterior wall
window strip
broken window
shutter panel
buttress
awning
roof edge
roof corner
roof center
roof damage
rooftop vent
rooftop antenna
rooftop AC / exchanger
catwalk
railing
stair
ramp
loading bay door
cargo shutter
checkpoint booth
fence straight
fence corner
fence gate
floodlight pole
utility mast
4H. Furniture and Room Objects
Small prop standard
48 x 48 px
Medium prop standard
96 x 96 px
Tall prop standard
96 x 144 px
Habitation sheet

Contents:

bed
bunk
locker
footlocker
table
desk
chair
bench
shelf
cabinet
sink
toilet
shower
galley counter
personal trunk
wall lamp
standing lamp
divider
Industrial utility sheet

Contents:

tool bench
tool rack
spare parts bin
generator
battery bank
breaker box
oxygen tank rack
gas canister rack
pipe valve
pump
compressor
conduit node
vent fan
coolant unit
hose reel
pallet
pallet jack
cargo trolley
scaffold
ladder
Medical sheet

Contents:

med bed
surgery bed
med cabinet
med cart
monitor stand
decon station
specimen fridge
IV stand
scrub sink
casualty bag
Command sheet

Contents:

command chair
tactical table
wall screen
comms rack
holo projector
secure locker
server cabinet
briefing table
4I. Interactable Objects

These must be extremely readable.

Small interactable standard
48 x 48 px
Tall interactable standard
48 x 96 px
Console cluster standard
96 x 96 px
Every interactable requires these state variants
default
highlight
active
inactive
damaged
repaired
objective-active
If lockable
locked
unlocked
If openable
closed
open
Required interactable sheets
Terminal / console sheet

Contents:

service terminal
diagnostic terminal
command terminal
helm terminal
orbital navigation console
relay beacon console
docking console
wall data terminal
portable field terminal
dead terminal shell
Container sheet

Contents:

cargo crate
med crate
weapons crate
personal locker
secure locker
service locker
sealed case
open versions
looted versions
Access / system sheet

Contents:

door panel
breaker switch
fuse box
keypad
card reader
intercom
alarm pull
boarding switch
airlock control
docking clamp console
relay power regulator housing
4J. Ship Exterior Pack

For alpha, one ship class only.

Shuttle exterior standards
component pieces: 96 x 96 px
larger hull chunks: 128 x 128 px
full shuttle top-down profile: 256 x 256 px
damaged shuttle / wreck profile: 256 x 256 px
source files: 2x
Required shuttle exterior sheet contents
nose / cockpit
canopy
central hull
port side pod
starboard side pod
dorsal plating
ventral plating
docking collar
side hatch
landing gear deployed
landing gear retracted
main thruster
maneuvering thrusters
radiator
sensor mast
nav lights
hull damage overlays
scorch overlays
shuttle shadow
wreck chunks
Required ship system icon sheet
reactor
battery
life support
sensors
comms
docking clamp
thrusters
nav core
cargo
damage control
hull integrity
fuel / propellant
atmosphere

Icons:

64 x 64 px
UI downscale target: 32 x 32
4K. Orbital / Space Layer Pack
Small orbital object
64 x 64 px
Medium orbital object
96 x 96 px
Large station / platform chunk
128 x 128 px
Background module
512 x 512 px tileable space/background plates
Required sheets
Orbital markers and navigation sheet

Contents:

route arrow
destination ping
objective ping
debris hazard
radiation hazard
no-go marker
dock-ready marker
relay marker
transfer marker
return-route marker
ship position marker
Space clutter sheet

Contents:

debris chunk small
debris chunk medium
wreck panel
cargo container drifting
beacon buoy
traffic marker
relay mast silhouette
docking arm
platform exterior chunk
station truss
shadow overlay
warning flare light
4L. Second-Destination Relay Pack
Standard
floor: 48 x 48
wall: 48 x 96
large relay structure: 96 x 144
exterior mast/tower: 128 x 192
Required sheets
Frontier landing pad sheet

Contents:

landing pad center
landing pad edge
landing pad corner
warning stripe overlays
landing numbers
pad lights
docking guide marks
Frontier relay structure sheet

Contents:

relay mast
signal dish
beacon tower
control room shell
storage room shell
service corridor shell
comms room shell
repair bay shell
cable run
coolant tanks
scaffold
relay signage
faction signage
generator
observation window
4M. Hazards, Combat FX, and Utility FX
FX frame standard
small FX: 48 x 48
medium FX: 64 x 64
large FX: 96 x 96
Hazard FX sheet

Contents:

oxygen leak plume
vacuum spray
radiation shimmer
toxic spill bubble
toxic drip
electric arc
sparks
smoke
steam
coolant spray
decon mist
fire small
fire medium
Combat FX sheet

Contents:

pistol muzzle flash
rifle muzzle flash
shotgun muzzle flash
tracer
metal hit spark
armor hit spark
flesh hit
ricochet
dirt puff
concrete puff
melee swing arc
stun strike
heal pulse
explosion small
explosion medium
Utility FX sheet

Contents:

scanner ping
terminal boot glow
objective complete ping
boarding confirmation
docking lock effect
undocking release effect
thruster flare
RCS puff
4N. Decals and Storytelling Overlays
Decal standard
small decal: 48 x 48
medium decal: 96 x 96
long strip decal: 48 x 96 or 96 x 48
Required contents
scorch marks
bullet holes
dented panel
rust streak
grime
oil stain
pooled fluid
dried blood
fresh blood
patch plate
warning stripe worn
serial number stencil
graffiti
floor arrow
quarantine marking
decon marking
maintenance text
cargo ID marks
faction logos
4O. Item Icons
Standard
icon export: 64 x 64
source size: 128 x 128
UI small version derived from same master: 32 x 32
Required icon sheets
Weapons
pistol
heavy pistol
SMG
shotgun
rifle
stun baton
knife
improvised melee
ammo / cell types
Tools
scanner
datapad
repair tool
welder
crowbar
cutter
sealant gun
extinguisher
beacon module
Armor / clothing
light suit
hazard suit
armored vest
coat
helmet
rebreather
goggles
gloves
boots
backpack
Resources / consumables
med injector
stim
ration
water
oxygen canister
battery cell
scrap
wiring bundle
spare parts
sealant cartridge
data chip
access badge
keycard
mission package
Ship / mission items
nav chip
relay component
docking authorization
power regulator
fuel canister
comms module
sensor package
black box
cargo manifest
4P. UI and HUD Art
Core sizes
small UI icon: 32 x 32
large UI icon: 64 x 64
cursor icons: 32 x 32
portrait frame: 256 x 256
panel corners built on 16 px grid
nine-slice panels: base sizes 128 x 128, 256 x 256
Required UI sheets
HUD icon sheet

Contents:

health
armor
oxygen
radiation
toxin
equipped weapon
equipped tool
mission objective
inspect
interact
loot
hostile
neutral
friendly
shipboard
orbital
docked
landed
outbound
return route
save/sync
warning
critical
Cursor sheet

Contents:

move
inspect
interact
attack
invalid
board
terminal
loot
drag / pan
Map/objective marker sheet

Contents:

player marker
objective marker
secondary marker
ship marker
dock marker
exit marker
hazard marker
loot marker
terminal marker
relay marker
Panel/frame sheet

Contents:

tooltip frame
mission panel
inventory panel
ship state panel
dialogue panel
terminal panel
notification strip
modal frame
tab sets
button states
4Q. Portraits and Comms
Portrait standard
in-game portrait: 256 x 256
source: 512 x 512
bust framing from chest up
neutral background plate provided separately
Required portrait categories
player modular bust set
dockworker
mechanic
medic
security
pilot
relay technician
trader
mission contact
raider
frontier operator
wounded survivor
Portrait variant states
neutral
alert
injured
masked
helmeted
comms overlay version
5. Deliverables by Phase
Current “fill out the game now” phase

Must be built first:

modular human base system
one hostile faction gear pass
one friendly worker/security pass
one terrain exterior family set
one facility interior set
one ship interior set
one relay destination set
one interactable sheet
one prop sheet
one hazard/combat FX sheet
one item icon sheet
one UI/HUD sheet
one portrait starter set
Alpha-complete phase

Add:

full animation completion
full faction variants
drone/turret suite
full decal library
full shuttle exterior system set
full orbital objects and markers
full second-destination relay pack
complete UI panel pass
complete portrait/comms roster

That priority matches the documented alpha slice and the project’s scope discipline around one planetary location, one orbital layer, one second destination, one ship class, one mission chain, and one complete planet-to-space loop before broader expansion.

6. Definition of Done for the Lead Artist

No asset is approved unless it includes:

final PNG export
layered source file
naming to standard
pivot/origin documented
state variants included
preview sheet included
one in-engine or mock placement test
one readability check at 100% scale
one dark-background and light-background check
one “crowded scene” readability check

No asset may be approved if:

silhouette is unclear
state is ambiguous
material is unreadable
scale is inconsistent
pivot is undocumented
atlas spacing is wrong
it only looks good zoomed in
it looks too “cool” but not useful in play
7. Naming Convention

Use this exactly:

WAR_[category]_[set]_[object]_[state]_[dir]_[frame]

Examples:

WAR_char_player_torso_security_idle_N_01
WAR_prop_terminal_diagnostic_active
WAR_tile_terrain_dustplain_outercorner_NE
WAR_fx_hazard_electricarc_03
WAR_ui_icon_oxygen_warning
8. Copy-Paste Prompt for the Lead Artist

Use this as the master brief.

Lead Artist Prompt — Wrath and Ruin

You are the Lead Artist for Wrath and Ruin, a top-down hard-science-fiction RPG roguelike sandbox that preserves the soul of a persistent multiplayer MUD through a tactical graphical client. The game must prioritize top-down readability, strong silhouettes, grounded hard-sci-fi materials, and immediate gameplay clarity over ornamental detail. The game is human-only in character fantasy; nonhuman silhouettes come from drones, turrets, bots, machinery, ships, and industrial infrastructure. The art must support a narrow alpha slice: one planetary location, one orbital layer, one second destination, one ship class, one mission chain, one authored encounter set, and one complete planet-to-space loop. Text, inspect output, logs, terminals, and environmental storytelling remain first-class identity pillars.

Visual rules

Style: grounded frontier hard-sci-fi
Camera: top-down tactical, not isometric diamond
Readability first at 100% gameplay scale
Strong silhouette separation for player, NPCs, hazards, terminals, containers, ship systems, and terrain states
Materials: worn metal, painted steel, polymer, reinforced glass, hazard paint, dust, ferrocrete, cables, sealants, insulation, grating
Palette: industrial neutrals first, controlled accents second, warning colors only where functional
No alien fantasy, no glossy space opera, no neon overload, no muddy over-rendered sprites

Technical rules

1 gameplay tile = 48x48 px export
Source art may be 2x, but final exports must be native size
Terrain/floors: 48x48
Walls/doors/hatches: 48x96
Characters: 64x96 per frame, 8 directions, center-bottom pivot
Small props: 48x48
Medium props: 96x96
Tall props: 96x144
Small drones: 48x48
Medium drones/turrets: 64x64
Shuttle exterior components: 96x96 and 128x128
Full shuttle top-down profile: 256x256
Item icons: 64x64
UI icons: 32x32 and 64x64
Portraits: 256x256 from 512x512 source
Atlas max size: 2048x2048
Padding between sprites: 4 px
Deliver PNG export plus layered PSD/Aseprite source plus preview/contact sheet

Character system
Build a modular human character kit using aligned 64x96 frames for:

base bodies
heads
hair
facial hair
helmets
face gear
torsos
arms
gloves
legs
boots
packs
weapons
tools

Build character variants for:

dockworker
mechanic
medic
security
pilot
relay tech
trader
settler
mission contact
raider
scavenger gunman
looter
armored enforcer
frontier ambusher

Mandatory animation sets:

idle
walk
interact/use
inspect
hit react
death/downed
Alpha-complete additions:
run
aim
fire
reload
melee
loot pickup
board/disembark
haul/carry

Terrain and architecture
Build:

exterior dust, dirt, rocky, gravel, ferrocrete, landing pad, hazard spill families
every terrain family must include fills, alternates, edges, outer corners, inner corners, diagonals, transition strips, isolated patch, damaged tile, shadowed tile
facility interiors with floor families, walls, corners, T-junctions, support columns, doors, blast doors, hatches, airlocks
ship interiors with compact deck floors, bulkheads, hatches, viewports, damaged hull pieces
frontier relay destination with landing pad, relay mast, beacon tower, control room, comms room, repair bay, storage, scaffolds, signage, generators

Props and interactables
Build readable stateful sheets for:

terminals
diagnostics consoles
helm consoles
orbital navigation consoles
relay beacon consoles
cargo crates
med crates
lockers
secure lockers
access panels
fuse boxes
keypads
card readers
intercoms
alarm stations
docking controls
airlock controls

All interactables must include state variants where applicable:

default
highlighted
active
inactive
locked
unlocked
open
closed
damaged
repaired
objective-active
exhausted/used

Ship and space
Build one alpha ship class with:

shuttle top-down exterior
hull components
cockpit/canopy
docking collar
landing gear deployed/retracted
thrusters
radiators
nav lights
damage overlays
wreck chunks

Build orbital visuals for:

debris
cargo drift
beacon buoys
relay/platform silhouettes
docking arms
traffic markers
route arrows
destination pings
hazard markers
return-route markers

Effects
Build hazard and combat FX for:

oxygen leak
vacuum spray
toxic spill
radiation shimmer
electric arc
sparks
smoke
steam
fire
muzzle flashes
tracers
armor/flesh hit FX
ricochets
melee swing
scanner ping
docking / boarding confirmation
thruster flares

UI and portraits
Build:

HUD icons
cursor states
mission markers
ship-state icons
inventory icons
terminal frames
mission panels
tooltips
notification bars
character portraits
NPC comms portraits

Approval standard
Nothing is approved unless it is readable in gameplay, aligned to its pivot, includes the correct state variants, and comes with source plus final export plus preview sheet.