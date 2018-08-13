# Cuppa

C++ prototyping facilities (simple media management). Windows specific until further notice.

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

  +-------+                           +------------+
  | MODEL |<----------updates---------| CONTROLLER |
  +-------+<-+                     +->+------------+
             |              writes/|     |
        reads|          manipulates|     |warn of a change
             |                     |     |(event)
             |     +------+--------+     |
             +-----| VIEW |<-------------+
                   +------+

### Game references

#### C64/Amiga Games

- Pipe mania
- Qix
- C64 Hacker
- Sokoban
- Super sprint
- Outrun
- Cinemaware: TV Sport/It cames from the desert/Rocket ranger/DSefender of the crown
- Sorcery+/Cauldron
- H.E.R.O
- Kill watt
- TrailBlazzer
- pengo
- Pacman
- Druid
- Boulder Dash
- Detroit (jeu d'entreprise de voiture)
- Gauntlet
- Frogger
- Snake/Tron
- hat-trick
- lode runner
- lock'n'chase (mattel intellivision)

#### Colleco Games

Carnival
Deluxe galaga/ GORF / Zaxxon
Dig Dug
Lady Bug (Colleco)
Time Pilot
Q-bert

#### GBA Games

- Kuru Kuru Kururin
- Mr Driller

#### Modern Games

- Braid
- Fez => 2D/3D + alphabet arithmetic
- Super Hot => time mechanics
- Zachtronics: manuel PDF
- Jump step step...
- Lumo
- Agar.io
- Spelunkny
- Steamword Dig
- Echochrome / Monument valley => Optical illusions
- Celeste: multiple level of accessebility
- Axiom verge
- Bastion: Narration, world build under player feet
- Dicey Dungeons
- Dark souls/bloodborn: ghost/marks
- fairune
- witch & knight

#### Genres

- Match3
- CoreWar
- Flipper
- Tower defense
- Picross
- Tetris => Tricky Tower
- Classic boardgames: Go, Connect 4, Game of Hex, Tic tac toe
- Minesweeper
- Columns
- breakout: Arkanoid
- Pong => Wind Jammer
- Battle Royal
- Clicker

#### Features

- Collectables
- Signs & Feebacks
- ARG (augmented reality game)

#### Editors

- [TyranoBuilder](https://steamcdn-a.akamaihd.net/steam/apps/345370/ss_d61c7b4fdfea79150e593d99caf248283a754715.jpg?t=1471008910)
- [Articy](https://steamcdn-a.akamaihd.net/steam/apps/570090/ss_cc84d4f0fac55778a548e30af10a544fe56b6769.jpg?t=1514994067)
- [Smile Game Builder](https://steamcdn-a.akamaihd.net/steam/apps/256702501/movie480.webm?t=1512121109)

Solstice

[Back to README](../../README.md)