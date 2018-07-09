# Cuppa

C++ prototyping facilities (simple media management). Windows specific until further notice.

## Thinkings

### functions

println

**We need a logger!!**

random / noise

loadImage

image (drawImage)

rect:

- should support rounded corners
- should support both (center, size) and (topLeftCorner, bottomRightCorner) construction parameter

ellipse:

- should support both (center, size) and (topLeftCorner, bottomRightCorner) construction parameter

support for gamepad

support for time

support for curve (bezier, spline...)

#### text utilities

loadcsv

loadjson

loadxml

### events

keyPressed

mousePressed

keyReleased

mouseReleased

### built in variables

mousex, mousey

pmousex, pmousey

width, height

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

[Back to README](../../README.md)