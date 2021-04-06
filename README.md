# C-MatrixLib

```c++
#include <iostream>

#include "matrix.h"

using namespace std;

int main() {
	/*
	System of Equations:
	 4w + 1x + 2y - 3z= -16 
	-3w + 3x - 1y + 4z=  20 
	-1w + 2x + 5y + 1z=  -4 
	 5w + 4x + 3y - 1z= -10
	
	A = [ [ 4, 1, 2, -3 ], [ -3, 3, -1, 4 ], [ -1, 2, 5, 1 ], [ 5, 4, 3, -1 ] ]
	B = [ [ -16 ], [ 20 ], [ -4 ], [ -10 ] ]
	
	A x C = B
	
	We want to find 'C'
	
	C = ~A x B
	
	'C' is equal to the inverse
	if 'A' dot multiplied by 'B'
	*/
	
	matrix<> A = {
		{  4, 1,  2, -3 },
		{ -3, 3, -1,  4 },
		{ -1, 2,  5,  1 },
		{  5, 4,  3, -1 }
	};
	
	matrix<> B = {
		{ -16 },
		{  20 },
		{  -4 },
		{ -10 }
	};
	
	A.inverse();
	
	matrix<> C = A * B;
  
	cout << C.toString() << endl;
	
	// Output:
	// [ [ -1 ], [ 1 ], [ -2 ], [ 3 ] ]
	// W = -1 ; X = 1 ; Y = -2 ; Z = 3 
}
```
