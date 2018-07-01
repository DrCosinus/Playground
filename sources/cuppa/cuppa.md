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

W * X = Y

[[ w11 w12]    [[x1]     [[w11*x1+w12*x2]    [[y1]
 [ w21 w22]] *  [x2]]  =  [w21*x1+w22*x2]] =  [y2]]

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