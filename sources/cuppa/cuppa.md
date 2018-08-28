# Cuppa

C++ prototyping facilities (simple media management). Windows specific until further notice.

## bulk

- imaging: createImage, loadPixels, updatePixels
- Point semantic => Corner, Center: rect(Corner topLeft, Corner bottomRight), rect(Center, Size)
- PointDifference semantic => Size, Offset, Direction
- split unit.hpp => Pixel, Point, Direction, Color 

{
    auto pixels = lockPixels{img};
    pixels[index] = Red;
}
image(img);

push(transform{});
pop(transform{});
append(translation{});
append(rotation{});
append(scale{});
append(shear{});
prepend(...);
reset(transform{});

generate(random());
generate(noise());

create(Image());


begin /end shape + vertex

set(fill(color));
set(stroke(color, thickness));
push(stroke{});
push(fill{});
set(font);
draw(ellipse(center, radius));
draw(rectangle(topLeftCorner, bottomRightCorner));
draw(rectangle(topLeftCorner, size));
draw(rectangle(center, size));
clear(background(color));

struct Center : Point
{
    using Point::Point;
    explicit(const Point& pt) : Point(pt);
};

struct ellipse
{
    ellipse(Center,Diameter);
};


20, 20; 80, 20; 50, 50

## Functions

### Input

#### Gamepad

support for gamepad

[XInput and DirectInput](https://docs.microsoft.com/en-us/windows/desktop/xinput/xinput-and-directinput)

#### Mouse

- [ ] getMouseX
- [ ] getMouseY
- [ ] getMouseWheel
- [ ] getMousePreviousX
- [ ] getMousePreviousY
- [ ] event mouseClicked(mousePosition, buttonStates)
- [ ] event mouseMoved()
- [ ] event mouseWheel()
- [ ] event mousePressed()
- [ ] event mouseReleased()
- [ ] event mouseDragged()

#### Keyboard

- [ ] event keyPressed(keyCode)
- [ ] event keyReleased(keyCode)

support for time

support for curve (bezier, spline...)

#### Input Files

- [ ] loadCSV
- [ ] loadJSON
- [ ] loadXML
- [ ] loadBytes
- [ ] loadStrings
- [ ] parseJSON
- [ ] parseXML

#### Time & Date

- [ ] day()
- [ ] hour()
- [ ] milliseconds()
- [ ] minute()
- [ ] month()
- [ ] second()
- [ ] year()
- [ ] now()

### Output

#### Text

- [ ] println
- [ ] print
- [ ] printArray

#### Output Images

- [ ] save

#### Output Files

- [ ] saveBytes
- [ ] saveCVS
- [ ] saveJSON
- [ ] saveStrings
- [ ] saveTable
- [ ] saveXML

**We need a logger!!**

### Math

#### Arithmetic

- [ ] abs
- [ ] ceil
- [ ] constrain
- [ ] dist
- [ ] exp
- [ ] floor
- [ ] lerp
- [ ] log
- [ ] mag
- [ ] map
- [ ] max
- [ ] min
- [ ] norm
- [ ] pow
- [ ] round
- [ ] sq
- [ ] sqrt

#### Trigonometry

- [x] Angle
- [ ] cos
- [ ] sin
- [ ] tan
- [ ] acos
- [ ] asin
- [ ] atan
- [ ] atan2

#### Random

- [x] random
- [x] randomGaussian
- [ ] noise

[Perlin noise](https://en.wikipedia.org/wiki/Perlin_noise)

### Images

- [x] loadImage
- [x] image
- [ ] createImage
- [ ] pixels
- [ ] updatePixels

### Color

- [x] background
- [x] fill(Color/Appliance)
- [x] stroke(Color/Thickness/Appliance)

### Shape

#### 2D Primitives

- [x] arc
- [x] ellipse
- [x] line
- [x] point
- [x] quad
- [x] rect
- [x] triangle

Pseudo types: Center, Corner to allow functions overloads rect(Corner, Corner) or rect(Center, Size), rect(Corner, Size)


rect:

- should support rounded corners
- should support both (center, size) and (topLeftCorner, bottomRightCorner) construction parameter

ellipse:

- should support both (center, size) and (topLeftCorner, bottomRightCorner) construction parameter

#### Curves

- [ ] bezier
- [ ] curve

#### 3D Primitives

- [ ] box
- [ ] sphere

#### Vertices

...

### lib to be considered for integration

sqlite

### math utilities

Vector2d, Vector3d
Matrix2d (Rows by Columns)

#### Linear algebra

for neural network study

$
\begin{alignedat}{3}
& W: \text{Weight matrix} & B: \text{Bias vector} \\
& X: \text{Input vector}  & Y: \text{Output vector}
\end{alignedat}
$

$W.X+B=Y$

$
\begin{bmatrix} w_{11} & w_{12} \\ w_{21} & w_{12} \\ w_{31} & w_{32} \end{bmatrix}
\begin{bmatrix} x_{1} \\ x_{2} \end{bmatrix}
+
\begin{bmatrix} b_{1} \\ b_{2}\\ b_{3} \end{bmatrix}
= \begin{bmatrix} w_{11}x_{1}+w_{12}x_{2}+b_{1} \\ w_{21}x_{1}+w_{22}x_{2}+b_{2} \\ w_{31}x_{1}+w_{32}x_{2}+b_{3} \end{bmatrix}
= \begin{bmatrix} y_{1} \\ y_{2} \\ y_{3} \end{bmatrix}
$

### Neural network Facilities

### minimal media interface (graphics, inputs,...)

## Appendices

### transform matrix

Generic:
$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} a & d & 0 \\ b & e & 0 \\ c & f & 1\end{bmatrix}
= \begin{bmatrix} a.x+b.y+c & d.x+e.y+f & 1\end{bmatrix}
$

#### Translation

Translate from offset $\begin{pmatrix} d_x & d_y \end{pmatrix}$:

$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ d_x & d_y & 1\end{bmatrix}
= \begin{bmatrix} x+d_x & y+d_y & 1\end{bmatrix}
$

#### Rotation

Rotate of angle $\alpha$:

$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} cos(\alpha) & sin(\alpha) & 0 \\ -sin(\alpha) & cos(\alpha) & 0 \\ 0 & 0 & 1\end{bmatrix}
= \begin{bmatrix} x.cos(\alpha)-y.sin(\alpha) & x.sin(\alpha)+y.cos(\alpha) & 1\end{bmatrix}
$

#### Scale

Scale by $\begin{pmatrix} s_x & s_y \end{pmatrix}$:

Scale:
$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & 1\end{bmatrix}
= \begin{bmatrix} s_x.x & s_y.y & 1\end{bmatrix}
$

#### Shear on X axis

Factor is $sh_x$:

$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} 1 & 0 & 0 \\ sh_x & 1 & 0 \\ 0 & 0 & 1\end{bmatrix}
= \begin{bmatrix} x+sh_x.y & y & 1\end{bmatrix}
$

#### Shear on Y axis

Factor is $sh_y$:

$
\begin{bmatrix} x & y & 1\end{bmatrix}
\begin{bmatrix} 1 & sh_y & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1\end{bmatrix}
= \begin{bmatrix} x & y+sh_y.x & 1\end{bmatrix}
$

### Camera transform

ObjectToWorld x World ToCam

### Bezier curve

#### Quadratic

$t \in [0, 1]$

$t^2P_0+2t(1-t)P_1+(1-t)^2P_2$

$t^2P_0+2tP_1-2t^2P_1+P_2-2tP_2+t^2P_2$

$t^2(P_0-2P_1+P_2)+2t(P_1-P_2)+P_2$

$M(t) = t^2(P_0-2P_1+P_2)+2t(P_1-P_2)+P_2 = at^2+bt+c$

$M(t+\delta t) = a(t+\delta t)^2+b(t+\delta t)+c$

$M(t+\delta t) = at^2+2at.\delta t+a.\delta t^2 + b.t + b.\delta t + c$

$M(t+\delta t) = M(t) + \delta t(2at + a.\delta t + b)$

$\dfrac{\delta M(t)}{\delta t} = 2at + a.\delta t + b$

## TODO

### Tech features

- Model View Controller (MVC)
- Finite State Machine (CompileTime & Coordinated)
- Entity Components System (CompileTime)
- Gamepad (XInput & DInput)
- Pathfind (A* & Navmesh)
- Collision & Physics
- Network & sockets, multi local
- Debug/Log (format{})
- Math: 3D Vector & Matrices
- Generation: Perlin noise
- Toggle fullscreen
- Store and restore last window position
- Retrieve data folder location
- Prodecural design facilities
- Determinism facitities
- Buildtask: Intermediary Folder management

#### Model View Controller

```plaintext
  +-------+                           +------------+
  | MODEL |<----------updates---------| CONTROLLER |
  +-------+<-+                     +->+------------+
             |              writes/|     |
        reads|          manipulates|     |warn of a change
             |                     |     |(event)
             |     +------+--------+     |
             +-----| VIEW |<-------------+
                   +------+
```

### Prototypes

- [ ] Deminer
- [ ] Frogger
- [ ] Frog'n'bog
- [ ] Tetris
- [ ] Columns
- [ ] Snake
- [ ] Arkanoid
- [ ] Sokoban/Box

### Game references

#### Mattel Intellivision

- Burger time
- Frog'n'bog
- Lock'n'chase
- Shark shark
- Venture

#### Colleco Games

- Carnival
- Dig Dug
- Donkey Kong
- Donkey Kong Jr
- GORF
- Lady Bug
- MrDo
- Q-bert
- Smurf
- Time Pilot
- Turbo
- Zaxxon

#### C64

- Attack of the mutant castle
- Aztec CHallenge
- Barbarians
- Bard's tale
- Beach Head
- Bomb jack
- Boulder Dash
- Bruce Lee
- Cauldron
- Centipede
- Choplifter
- Commando
- Decathlon
- Deluxe galaga
- Druid
- Ego
- Frogger
- Gauntlet
- Hacker
- Hard ball
- hat-trick
- H.E.R.O
- Impossible Mission
- Jet set willy
- Karateka
- Klax
- Kill watt
- Law of the West
- Last Ninja
- Last V8
- LCP
- Lemmings
- lode runner
- Manic miner
- Marble Madness
- Master of the lamp
- Nebulus
- Miner 1049er
- Indiana Jones (Fate of atlantis, Last Crusade)
- One on One
- Outrun
- Pacman
- Paper boy
- Paranoid
- pengo
- Pipe mania
- Pirate (sid meyer)
- Pitfall
- Pole Position
- Puzznic
- Qix
- Snake/Tron
- Sokoban
- Solo Flight
- Sorcery+
- Spy hunter
- Spy vs Spy
- Summer Games (winter, california, world)
- Super sprint
- TrailBlazzer
- Uridium

#### Amiga

- 10th frame
- Al quadim
- Another world
- Battle Squadron
- Bubble bobble
- Cinemaware: TV Sport/It cames from the desert/Rocket ranger/DSefender of the crown
- Detroit (jeu d'entreprise de voiture)
- Dungeon master
- Dynablaster (bomberman)
- Eye of th e beholder
- Flash back
- Hang out
- Hybris
- Killing Game Show
- LeaderBoard
- Les Voyageurs du temps
- Lotus Turbo Esprit
- Operation Stealth
- Pang
- Populus
- Port of Call
- Powerplay: the game of the gods
- Prince of Persia
- New zealand story
- Nitro
- Pinball Dreams/Illusion/Fantasy
- Rainbow Islands
- Shadow of the beast
- Speed Ball 2
- Super Cars
- Turrican
- UFO
- Wizball
- Xenon 2

#### GBA Games

- Kuru Kuru Kururin
- Mr Driller

#### Modern Games

- Agar.io
- Axiom verge
- Bastion: Narration, world build under player feet
- Braid
- Celeste: multiple level of accessebility
- Dark souls/bloodborn: ghost/marks
- Dicey Dungeons
- Echochrome / Monument valley => Optical illusions
- fairune
- Fez => 2D/3D + alphabet arithmetic
- Jump step step...
- Lumo
- Okami
- Ratchet'n'Clank serie
- shadow of the colossus/ico
- Spelunkny
- Steamword Dig
- Super Hot => time mechanics
- Super Meat Boy
- witch & knight

#### Genres

- action adventure
- Battle Royal
- breakout: Arkanoid
- Classic boardgames: Go, Connect 4, Game of Hex, Tic tac toe
- Clicker
- Columns
- CoreWar
- Flipper
- Igavania
- Japanese Role Playing Game
- Match3
- Minesweeper
- patchinko
- platform
- Picross
- Programmation
- Pong => Wind Jammer
- Real Time Strategy
- Roguelike
- Roguelite
- Role Playing Game
- shmup
- Tetris => Tricky Tower
- Tower defense
- Zachtronics: manuel PDF

#### Features

- Collectables
- Signs & Feebacks
- ARG (augmented reality game)
- Skill tree
- Mode photo

#### Editors

- [TyranoBuilder](https://steamcdn-a.akamaihd.net/steam/apps/345370/ss_d61c7b4fdfea79150e593d99caf248283a754715.jpg?t=1471008910)
- [Articy](https://steamcdn-a.akamaihd.net/steam/apps/570090/ss_cc84d4f0fac55778a548e30af10a544fe56b6769.jpg?t=1514994067)
- [Smile Game Builder](https://steamcdn-a.akamaihd.net/steam/apps/256702501/movie480.webm?t=1512121109)

Solstice

[Back to README](../../README.md)