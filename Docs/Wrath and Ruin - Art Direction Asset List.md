# Wrath and Ruin - Art Direction Asset List

Status:

- this document is a broad backlog and intake reference
- this document is not the only technical source of truth

Use these documents first when there is any ambiguity:

- `Docs/Wrath and Ruin - Art Audit and Direction Lock.md`
- `Docs/Wrath and Ruin - Lore and Visual Theme Guide.md`
- `Docs/Wrath and Ruin - Lead Artist Onboarding Package.md`
- `Docs/Wrath and Ruin - Lead Artist Considerations.md`

The list below remains useful as a backlog, but measurements, camera rules, export requirements, and runtime-integration constraints are locked in the onboarding package.

1. Modular human character system

This is the single most important asset family.

1A. Base body spritesheets

Needed now and for alpha.

You want these separated so customization and faction variation stay cheap:

heads
hair
facial hair
helmets
visors / masks / respirators
torsos
chest armor / rigs
arms / sleeves
hands / gloves
legs / pants
boots
backpacks / tanks / back gear
belts / pouches / holsters
shoulder gear / pads
weapon-holding hands
tool-holding hands
1B. Head sheet contents

At minimum:

bald male
bald female
bald androgynous
short hair variants
tied-back hair variants
long hair variants
shaved sides / undercut
buzz cut
older face variants
scarred face variants
cyber-eye / prosthetic face variants
light / medium / dark skin-tone sets
neutral face
grim face
hurt face
dead / unconscious face
1C. Hair / helmet / face gear sheet
crew cut
messy short
ponytail
bun
cropped curls
braids
work cap
headset
soft hood
utility helmet
combat helmet
pressure hood
rebreather mask
respirator mask
visor up
visor down
goggles
face shield
officer cap
pilot helmet
1D. Torso / armor / clothing sheet
undersuit
light utility suit
engineer suit
medic torso
dockworker torso
scavenger jacket
security vest
raider chest rig
hazard / decon torso
pilot torso
relay operator torso
civilian coat
armored coat
heavy vest
long coat / duster
poncho / weather wrap
1E. Legs / boots sheet
utility pants
fatigues
armored pants
work pants
hazard suit legs
civilian trousers
pilot legs
heavy boots
work boots
soft shoes
armored greaves
knee pads
1F. Arms / gloves sheet
bare forearms
long-sleeve utility arms
armored forearms
medical sleeves
hazard sleeves
fingerless gloves
work gloves
tactical gloves
insulated gloves
1G. Back gear sheet
no pack
satchel
backpack
field pack
oxygen tank
tool rig
med pack
comms pack
power pack
cargo sling
1H. Weapon sheet

You will want separate hand-held weapons, not baked into the body.

pistol
revolver / heavy sidearm
SMG
shotgun
compact carbine
long rifle
battle rifle
stun baton
combat knife
crowbar
shock prod
improvised club
empty hands / fists
1I. Tool sheet
scanner
datapad
keycard / access fob
med injector
repair tool
welding torch
cutting torch
wrench
pry tool
sealant gun
sampling tool
cargo hook
beacon module
extinguisher
1J. Status / overlay sheet
selection ring
objective marker ring
friendly / neutral / hostile ring
low oxygen overlay
radiation exposure overlay
toxic contamination overlay
suit damage overlay
armor break overlay
blood / injury overlay
stunned overlay
unconscious overlay
dead overlay
interact highlight
inspect highlight
1K. Animation clips for the modular human kit

These are not optional for alpha.

idle
alert idle
walk
run
crouch / low-ready
aim
fire single shot
fire burst
reload
melee strike
interact / use terminal
inspect
open crate / locker
loot pickup
hit react
stagger
downed
death
board / disembark
speak / emote
carry light object
drag / haul pose
2. Human faction and NPC visual packs

Needed now to make the world feel inhabited; required for alpha.

You do not need a massive roster. You need a small number of strong, readable human archetypes.

2A. Friendly / neutral humans
dockworker
mechanic
medic
security guard
pilot
relay technician
quartermaster
scavenger trader
wounded survivor
local settler / worker
admin / operator
mission contact
2B. Hostile humans
raider
scavenger gunman
desperate looter
armored enforcer
hazard-zone marauder
frontier ambusher
boarding thug
relay pirate / privateer
2C. Faction differentiation assets

Do this mostly with swaps, not full new characters:

faction palettes
shoulder insignia
helmet markings
chest rig variants
backpack variants
weapon family variants
stance / posture variants
headgear differences
icon / armband colors
3. Drone, turret, and machine actor pack

Because you are human-only, machines are your nonhuman silhouette set.

Needed now
service drone
damaged service drone
security drone
cargo bot
wall turret
floor turret
inactive machine shell
hacked / disabled version
Alpha additions
repair drone
sensor drone
docking utility bot
relay maintenance bot
shipboard helper bot

Animations:

idle hover / idle wheel
patrol move
aim / track
attack
spark damage
disable / collapse
repair / scan beam
4. Planetary terrain tilesheets

You asked specifically for plains, edges, corners, diagonals. Build your terrain this way.

4A. Exterior ground terrain families

For the current slice and alpha, I would make these terrain families:

dust plain
compacted dirt
rocky ground
gravel / scree
scrubland
dead grass patch
ferrocrete / industrial apron
landing pad plating
waste / stained ground
trench / dug ground
flattened / worked terrain
hazard-spill ground
4B. Every terrain family should include
main fill tile
3–6 alt fill tiles
north edge
south edge
east edge
west edge
outer corner NW / NE / SW / SE
inner corner NW / NE / SW / SE
diagonal edge variants
thin strip transition
peninsula / one-tile protrusion
isolated patch tile
damaged version
shadowed version
raised-lip / embankment version
overlay transition into neighboring terrain
4C. Natural clutter sheet
pebbles
small rock clusters
medium rocks
boulders
cracked ground decals
dry scrub
dead brush
wind-blasted shrub
sparse tree / dead tree if biome supports it
root / stump
trash drift
scrap heap
cable spill
wreck fragments
5. Interior floor and wall architecture tilesheets

You need separate sheets for readability.

5A. Industrial facility interior sheet
clean metal floor
worn metal floor
grated floor
hazard-striped floor
maintenance floor
decon floor
med floor
command floor
darkened / powerless floor
blood / fluid stained floor
straight wall horizontal / vertical
outer corners
inner corners
T-junctions
cross junction
short wall / half wall
broken wall
reinforced wall
window wall
blast door frame
open blast door
closed blast door
locked blast door
destroyed blast door
airlock frame
vent wall
pipe wall
cable wall
support column
5B. Ship interior sheet
ship deck floor
cockpit floor
engineering floor
cargo hold floor
bunk area floor
airlock floor
bulkhead straight
bulkhead corners
hatch frame
hatch open
hatch closed
ladder hatch
maintenance crawl hatch
window / viewport section
sealed compartment wall
damaged compartment wall
hull breach tile
emergency patch tile
5C. Frontier relay / outpost interior sheet
relay office floor
control room floor
old grating
patched floor
stained floor
habitation floor
storage floor
relay wall
patched wall
insulated wall
relay window
damaged window
access door open / closed / locked
service hatch
comms room wall panel
piping corner pieces
support braces
6. Exterior building tilesheets

Needed to stop the game feeling like rooms dropped on terrain.

6A. Building shell sheet
exterior wall straight
exterior corner
exterior inner corner
exterior damaged wall
reinforced wall
window strip
broken window
shutter panel
support buttress
awning
roof edge
roof corner
roof center
roof damage
rooftop vent
rooftop antenna
rooftop AC / exchanger
external ladder
catwalk
railing
stair / ramp modules
6B. Settlement / dock frontage sheet
loading bay door
cargo shutter
dock gate
barricade
checkpoint booth
fence straight
fence corner
fence gate
warning sign posts
floodlight pole
utility pole
docking mast
7. Furniture and room-object sheets

These make interiors readable and believable.

7A. Habitation / civilian
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
shower stall
galley counter
food storage
personal trunk
lamp
wall monitor
fan
room divider
potted hardy plant
7B. Industrial / utility
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
power conduit node
vent fan
coolant unit
hose reel
pallet
pallet jack
cargo trolley
scaffold
ladder
maintenance crane hook
7C. Medical
med bed
surgery bed
medicine cabinet
med cart
monitor stand
decon station
body bag / casualty bag
specimen fridge
IV stand
sink / scrub station
7D. Command / control
command chair
helm chair
nav console
wall screen
holo projector
tactical table
comms rack
server cabinet
secure locker
data core housing
8. Interactable object sheet

These are gameplay-critical.

State variants are mandatory

For every major interactable, plan the states:

idle
highlighted
powered
unpowered
damaged
repaired
objective-active
locked
unlocked
open
closed
used / exhausted
Objects
service terminal
diagnostic terminal
command terminal
helm terminal
orbital nav console
relay beacon console
docking console
access panel
fuse box
crate
cargo crate
med crate
weapons locker
personal locker
sealed locker
door panel
switch / breaker
keypad
card reader
intercom
alarm station
boarding ramp
docking clamp
airlock control
elevator / lift console if used
9. Ship asset pack

Very important for your Cosmoteer side of the fantasy.

9A. Ship exterior sheet

For alpha you only need one class, but it needs to read as a real ship.

shuttle hull main body
nose / cockpit canopy
side pod / cargo side
dorsal paneling
ventral paneling
airlock
docking collar
loading hatch
landing gear deployed
landing gear retracted
main thruster
small maneuvering thrusters
radiator
sensor mast
comms antenna
nav light set
hull damage decals
shadow
wrecked shuttle pieces
9B. Ship interior props
pilot seat
co-pilot seat
helm console
nav console
power panel
engine access panel
cargo net
tie-downs
spare parts locker
emergency med kit shelf
bunk
wall storage
emergency suit rack
internal hatch
maintenance crawl panel
9C. Ship system icons / sprites

For UI and readability.

reactor / power
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
10. Orbital / space environment pack

Needed by alpha because the slice includes orbital travel.

10A. Background and space-readability assets
planet limb / horizon variants
starscape layers
nebula haze very light
orbital grid / lane markers
route arrows
debris field tiles
wreck chunks
cargo containers drifting
beacon lights
comm relay structures
traffic buoys
docking arm
station silhouette
platform silhouette
shadow overlays
warning hazard overlays
10B. Space hazard / traversal markers
debris hazard marker
radiation zone marker
no-go / military lane marker
dock-ready marker
objective ping
destination ping
jump / transfer arrow
collision warning icon
11. Second-destination asset pack

For alpha this should be a distinct place, even if compact.

Dust Frontier / relay style pack
landing pad
docking collar
relay mast
signal dish
beacon tower
control room shell
storage room shell
service corridor shell
frontier bunks / habitation nook
comms room
repair bay
external cables
coolant tanks
scaffold
relay signage
faction signage
cargo pallets
emergency generator
observation window
pad lights
warning stripes / painted numbers
12. Hazard, VFX, and combat FX sheets

This is where the game gets its “alive” feeling.

12A. Environmental hazards
oxygen leak plume
vacuum vent spray
radiation shimmer
toxic spill puddle
toxic drip
electric arc
fire small
fire medium
sparks
smoke
steam
coolant spray
decon mist
12B. Combat FX
muzzle flash pistol
muzzle flash rifle
muzzle flash shotgun
projectile tracer
hit spark metal
hit spark armor
flesh hit
ricochet
bullet puff dirt
bullet puff concrete
melee swing arc
stun / electrical strike
heal / med inject effect
death collapse dust
explosion small
explosion medium
12C. Movement and interaction FX
footsteps dust
footsteps metal
door opening sparks / light
terminal boot glow
scan pulse
objective complete ping
boarding / docking confirmation effect
13. Damage, grime, decal, and storytelling overlays

These are cheap and hugely important.

scorch marks
bullet holes
dented panel decals
rust streaks
grime buildup
oil stains
pooled fluid
dried blood
fresh blood
hazard stripes worn
patch plates
warning text decals
serial numbers
graffiti
floor arrows
parking / landing pad numbers
corporate / faction logos
quarantine markings
decon markings
maintenance labels
14. Item icon atlas

Needed now for inventory credibility; required for alpha.

14A. Weapons
pistol
heavy pistol
SMG
shotgun
rifle
stun baton
knife
improvised melee
ammo / energy cell types
14B. Tools
scanner
datapad
repair tool
welder
crowbar
cutter
sealant gun
extinguisher
beacon module
14C. Armor / clothing
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
14D. Consumables / resources
med injector
stim
ration pack
water pack
oxygen canister
battery cell
scrap metal
wiring bundle
spare parts
sealant cartridge
data chip
access badge
keycard
mission package
14E. Ship / mission items
nav chip
relay component
docking authorization
power regulator
fuel canister
comms module
sensor package
black box
cargo manifest
15. UI and HUD asset set

These are assets too, not just code.

15A. Core UI frame set
panel backgrounds
bordered windows
buttons
tabs
checkboxes
dropdown / list components
scrollbar parts
tooltip box
modal frame
notification strip
dialogue / terminal frame
15B. HUD icon set
health
armor
oxygen
radiation
toxin
hunger / fatigue if used
weapon equipped
tool equipped
mission objective
inspect
interact
loot
talk
hostile
neutral
friendly
shipboard
orbital
docked
landed
outbound
return route
save / sync
warning / critical
15C. Input / cursor assets
move cursor
inspect cursor
interact cursor
attack cursor
invalid cursor
boarding cursor
terminal cursor
loot cursor
drag / pan cursor
selection marquee / ring
15D. Map / objective markers
player marker
objective marker
secondary objective marker
ship marker
dock marker
exit / transition marker
hazard marker
loot marker
terminal marker
relay beacon marker
16. Portrait and profile assets

Very important for the MUD soul and RPG identity.

Needed now
player bust portraits, modular from the same head / gear parts
NPC portrait busts for key mission contacts
faction badge icons
comms call portraits
wanted / target portraits if used
Needed by alpha
wounded / masked portrait variants
civilian / hostile portrait variants
ship contact portraits
relay contact portraits
dossier frame graphics
17. Text content assets

This is first-class content in your game, not filler.

Required now
terrain inspect text
prop inspect text
terminal short text
hazard warnings
mission objective text
event log text
combat narration text
item descriptions
equipment flavor text
location flavor text
ship descriptions
room / area names
signage text
Required by alpha
mission briefings
mission updates
success / failure text
NPC short barks
radio chatter text
shipboard logs
relay logs
environmental storytelling notes
onboarding / tutorial copy
emote support text
speech formatting assets
codex / glossary stubs if used
18. Audio asset backlog

Your docs explicitly call for ambience, hazard clarity, combat readability, and useful feedback sounds.

18A. Ambience
planetary wind
distant machinery
dock ambience
facility hum
medbay ambience
command room tone
ship interior hum
engine idle
orbital void tone
relay platform ambience
emergency alarm ambience
radiation-zone ambience
18B. Foley
footsteps on dirt
footsteps on metal
footsteps on grating
footsteps on concrete
container open / close
locker open / close
hatch open / close
door slide
keypad press
terminal typing / confirm
item pickup
inventory move
boarding ramp extend / retract
18C. Combat
pistol shot
rifle shot
shotgun shot
stun hit
melee hit
ricochet
impact flesh
impact armor
reload
dry fire
pain grunt
death fall
explosion small
explosion medium
18D. Hazard and system audio
oxygen leak
electric arc
radiation pulse
toxic bubble / hiss
fire crackle
extinguisher spray
scanner ping
mission complete chime
warning beep
critical alarm
docking lock
undocking release
thruster burn
RCS puff
nav confirm tone
19. Technical support assets

These are easy to forget and always bite later.

atlas metadata
sprite anchors / weapon sockets
animation event markers
collision masks
selection masks
emissive masks
shadow masks
damage masks
material / palette guides
faction color guides
prefab thumbnails
UI nine-slice assets
font assets
icon export source files
naming convention sheet
asset validation rules
LOD / size rules if needed
import presets
render-test scenes
20. What you need immediately to make the current game feel fuller

This is the shortlist I’d prioritize first:

one complete modular human kit
one hostile human kit
one friendly worker / security / technician kit
one service drone and one turret
one planetary exterior terrain sheet
one facility interior architecture sheet
one ship interior architecture sheet
one second-destination relay sheet
one prop / furniture sheet
one interactable sheet
one hazards / combat FX sheet
one item icon atlas
one UI HUD atlas
one portrait sheet
one text content pass for inspect / terminal / mission / combat
one audio pass for ambience / interaction / hazards / combat
21. What you need by alpha sign-off

This is the full alpha-complete minimum:

modular human customization pipeline
enough faction variants to make friend / foe / civilian roles readable
one planetary biome set
one orbital presentation set
one second-destination set
one shuttle class inside and out
one complete interactable object library
one complete furniture / room-object library
one hazard / combat / traversal VFX library
one item / equipment icon library
one usable UI / mission / inventory / ship-state icon library
portrait and comms presentation assets
text content pack strong enough to preserve the MUD identity
audio pack strong enough to make hazards, combat, ship state, and interactions readable
22. The cleanest production packaging for your art team

I would organize the visual backlog into these deliverable packs:

characters_humans_modular
characters_humans_factions
actors_drones_and_turrets
terrain_planet_surface
terrain_facility_interior
terrain_ship_interior
terrain_frontier_relay
structures_exterior_shells
props_furniture_and_rooms
props_interactables
ships_shuttle_khepri_family
space_orbital_layer
fx_hazards_and_combat
icons_items_and_equipment
ui_hud_and_panels
portraits_and_comms
text_content_pack
audio_core_pack

The main trap to avoid is making a lot of beautiful one-off pieces without first locking the modular human kit, terrain autotiling rules, interactable state variants, and ship readability kit. Those four systems will carry most of the game.

The next useful step would be turning this into a sprint-ready asset tracker with columns for sheet name, contents, priority, owner, reuse value, and alpha gate.
