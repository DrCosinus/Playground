# Cuppa

C++ prototyping facilities (simple media management). Windows specific until further notice.

## Thinkings

### text utilities

loadcsv
loadjson
loadxml

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

### transform matrix

```cpp
  // ObjectToWorld x World ToCam

  // Generic
  //          [[  a  d  0 ]
  //  [x y 1]  [  b  e  0 ]   =  [ a*x+b*y+c d*x+e*y+f 1 ]
  //           [  c  f  1 ]]

  // Translate
  //          [[  1  0  0 ]
  //  [x y 1]  [  0  1  0 ]   =  [ x+dx y+dy 1 ]
  //           [ dx dy  1 ]]

  // Rotate
  //          [[  cos(a) sin(a)  0 ]
  //  [x y 1]  [ -sin(a) cos(a)  0 ]   =  [ x.cos(a)-y.sin(a) x.sin(a)+y.cos(a) 1 ]
  //           [       0      0  1 ]]

  // Scale
  //          [[ sx  0  0 ]
  //  [x y 1]  [  0 sy  0 ]   =  [ x*sx y*sy 1 ]
  //           [  0  0  1 ]]

  // ShearX
  //          [[  1  0  0 ]
  //  [x y 1]  [  s  1  0 ]   =  [ x+s*y y 1 ]
  //           [  0  0  1 ]]
```

[Back to README](../../README.md)