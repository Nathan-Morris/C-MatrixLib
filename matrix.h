#include <iostream>
#include <sstream>
#include <initializer_list>

#pragma once

using std::cout;
using std::endl;
using std::initializer_list;

/*
Bit of a cheat here, strictly speaking it isnt a "const char*", the
function returns a dynamically allocated portion of memory which
acts similiarly to a 'const char*' as in it takes up memory for the 
duration of the programs runtime. Idealy this is called in conjunction 
with the 'matrix_exception::printException()' member or at the end of a 
programs runtime so while memory leaking would occur, the memory is
freed almost immediatly when the program exits.
*/
inline const char* integralToConstChar(long long l) {
	// '-9223372036854775808' : 'LLONG_MIN' -> 20 chars
	char buffer[21]; // needed additional 1 cell for '\0'
	memset(buffer, '\0', 21);

	sprintf_s(buffer, "%I64d", l);

	char* retBuffer = new char[21];
	memcpy(retBuffer, buffer, 21);

	return (const char*)retBuffer;
}

template<typename _NumberType>
inline _NumberType evaluateMatrixNumberGrouping(_NumberType* n1, _NumberType* n2, unsigned int len, bool performVectorDelete = false) {
	_NumberType n = 0;
	for (unsigned int i = 0; i < len; i++)
		n += (n1[i] * n2[i]);
	if (performVectorDelete)
		delete[] n1, n2;
	return n;
}

class matrix_exception : public std::exception {
private:
	
	const char* what;
	const char** info;
	unsigned short rows;
	unsigned short columns;

public:
	
	matrix_exception(
		const char* what, 
		unsigned short rows, 
		unsigned short columns, 
		const char** info = NULL
	);

	void printException(int exitCode = -1);
};

// _NumberType can be any base type or class with valid numerical operations implemented
// If a class is used for _NumberType it must have a '(*)(void)' constructor avaliable,
// and operator functions allowing initialization with integral and floating types for
// full support, I think, haven't tested it yet, call it a todo
template<typename _NumberType = float>
class matrix {

private:

	unsigned short rows;
	unsigned short columns;

	_NumberType** contents;

	// dynamic allocation, requires call to 'clean()' member
	void initializeContents(unsigned short rows, unsigned short columns) {
		this->contents = new _NumberType*[rows];
		for (unsigned short i = 0; i < rows; i++)
			this->contents[i] = new _NumberType[columns];
	}

	// INDEX not offset
	void emplaceRow(unsigned short rowIndex, _NumberType* dest) {
		for (unsigned short c = 0; c < this->columns; c++) {
			dest[c] = this->at(rowIndex, c + 1);
		}
	}

	// INDEX not offset
	void emplaceColumn(unsigned short columnIndex, _NumberType* dest) {
		for (unsigned short r = 0; r < this->rows; r++) {
			dest[r] = this->at(r + 1, columnIndex);
		}
	}

	matrix() {
		this->rows = 0;
		this->columns = 0;
		this->contents = NULL;
	}

public:

	matrix(unsigned short rows, unsigned short columns, _NumberType zeroEquivalent = 0) {
		this->rows = rows;
		this->columns = columns;
		this->initializeContents(rows, columns);
		this->fill(zeroEquivalent);
	}

	matrix(initializer_list<initializer_list<_NumberType>> list2d) : rows{ (unsigned short)list2d.size() }, columns{ 0 } {
		try {
			for (auto& elem : list2d) {
				if (this->columns == 0) {
					this->columns = elem.size();
				}
				else {
					if (this->columns != elem.size()) {
						throw matrix_exception(
							"Use Of Varied Column Lengths In Initializer List Prohibited",
							this->rows,
							this->columns
						);
					}
				}
			}

			typename initializer_list<initializer_list<_NumberType>>::iterator iter = list2d.begin();
			typename initializer_list<_NumberType>::iterator innerIter;

			this->initializeContents(this->rows, this->columns);

			for (unsigned int r = 0; r < this->rows; r++, iter++) {
				innerIter = (*iter).begin();
				for (unsigned int c = 0; c < this->columns; c++, innerIter++) {
					this->contents[r][c] = *innerIter;
				}
			}
		}
		catch (matrix_exception& e) {
			e.printException();
		}
		catch (...) {
			printf("Something Else Went Wrong, Idk How???\n");
			exit(-1);
		}
	}

	// handles 'contents' allocated memory, prevents memory leak
	void clean() {
		if (this->contents == NULL)
			return;
		for (unsigned short i = 0; i < this->rows; i++)
			delete[] this->contents[i];
		delete[] this->contents;
	}

	void fill(_NumberType value) {
		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				this->contents[r][c] = value;
			}
		}
	}

	// recursive (sort of), I am astonished I got it working
	_NumberType getDeterminant() {
		try {
			if (this->rows != this->columns) {
				throw matrix_exception(
					"Unable To Get Determinant Of A Non-Square Matrix",
					this->rows,
					this->columns
				);
			}

			if (this->rows == 1)
				return this->at(1, 1);

			_NumberType retVal = 0; // TODO, implement 'Zero Equivalent' for classes

			if (this->rows == 2) {
				// ad - bc
				return ((this->at(1, 1) * this->at(2, 2)) - (this->at(1, 2) * this->at(2, 1)));
			}

			for (unsigned short c = 0; c < this->columns; c++) {
				matrix<_NumberType> subMinor = this->getMatrixMinor(1, c + 1);
				retVal += this->at(1, c + 1) * subMinor.getDeterminant() * (c % 2 == 0 ? 1 : -1);
				subMinor.clean();
			}

			return retVal;
		}
		catch (matrix_exception& e) {
			e.printException();
			return 0;
		}
		catch (...) {
			printf("This is VooDoo Right?\n");
			exit(-1);
		}
	}

	void transpose() {
		matrix<_NumberType> mTrans(this->columns, this->rows);

		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				mTrans.at(c + 1, r + 1) = this->at(r + 1, c + 1);
			}
		}

		this->clean();
		this->contents = mTrans.contents;
	}

	// seems a bit unconventional / bulky, may need to improve this later
	matrix<_NumberType> getMatrixMinor(unsigned short rowIndexPos, unsigned short columnIndexPos) {
		rowIndexPos--;
		columnIndexPos--;

		matrix<_NumberType> retMatrix(this->rows - 1, this->columns - 1);

		_NumberType* singleDimCollection = new _NumberType[(this->rows - 1) * (this->columns - 1)];
		unsigned short sDCi = 0;

		for (unsigned short r = 0; r < this->rows; r++) {
			if (r == rowIndexPos)
				continue;
			for (unsigned short c = 0; c < this->columns; c++) {
				if (c == columnIndexPos)
					continue;
				singleDimCollection[sDCi++] = this->at(r + 1, c + 1);
			}
		}

		for (unsigned short r = 0; r < this->rows - 1; r++) {
			for (unsigned short c = 0; c < this->columns - 1; c++) {
				unsigned int pos = (r * (this->rows - 1)) + c;
				retMatrix.at(r + 1, c + 1) = singleDimCollection[pos];
			}
		}

		delete[] singleDimCollection;
		return retMatrix;
	}

	void minors() {
		matrix<_NumberType> mMinor(this->columns, this->rows);

		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				matrix<_NumberType> subMinor = this->getMatrixMinor(r + 1, c + 1);
				mMinor.at(r + 1, c + 1) = subMinor.getDeterminant();
				subMinor.clean();
			}
		}

		this->clean();
		this->contents = mMinor.contents;
		
		//cout << this->toString() << endl;
	}

	void cofactors() {
		this->minors();

		bool negFlag = false; //prevLine = false;
		for (unsigned short r = 0; r < this->rows; r++) {
			negFlag = prevLine;
			prevLine = !prevLine;
			for (unsigned short c = 0; c < this->columns; c++) {
				//cout << "<" << r << ":" << c << "> : " << negFlag << endl;
				if (negFlag) {
					this->contents[r][c] *= -1;
				}
				negFlag = !negFlag;
			}
		}

		//cout << this->toString() << endl;
	}

	void inverse() {
		try {
			if (this->rows != this->columns) {
				throw matrix_exception(
					"Unable To Inverse A Non-Square Matrix",
					this->rows,
					this->columns
				);
			}

			_NumberType determinant = this->getDeterminant();

			if (determinant == 0) {
				throw matrix_exception(
					"Unable To Inverse A Singular Matrix",
					this->rows,
					this->columns
				);
			}

			this->cofactors();

			this->transpose();

			this->operator*=(1 / determinant);
		}
		catch (matrix_exception& e) {
			e.printException();
		}
		catch (...) {
			printf("Oh No!!!, How'd We Get Here???\n");
			exit(-1);
		}
	}

	unsigned short rowsCount() { return this->rows; }
	unsigned short columnsCount() { return this->columns; };

	// get java'd, pt.2
	std::string toString() {
		std::stringstream ss;
		ss << '[';

		for (unsigned short r = 0; r < this->rows; r++) {
			ss << ' ' << '[';
			for (unsigned short c = 0; c < this->columns; c++) {
				ss << ' ' << this->at(r + 1, c + 1);
				if (c != this->columns - 1)
					ss << ',';
			}
			ss << ' ' << ']';
			if (r != this->rows - 1)
				ss << ',';
		}

		ss << ' ' << ']';

		return ss.str();
	}

	_NumberType& at(unsigned short rowIndex, unsigned short columnIndex) {
		// decrement due to difference in matrix index notation & array offset notation
		rowIndex--;
		columnIndex--;

		try {
			if (rowIndex >= this->rows) {
				throw matrix_exception(
					"Row Index Provided Invalid",
					this->rows,
					this->columns
				);
			}
			else if (columnIndex >= this->columns) {
				throw matrix_exception(
					"Column Index Provided Invalid",
					this->rows,
					this->columns
				);
			}
			else {
				return this->contents[rowIndex][columnIndex];
			}
		}
		catch (matrix_exception& e) {
			e.printException();
		}
	}

	template<typename _InNumberType>
	matrix<_NumberType> operator*(const _InNumberType& n) {
		matrix<_NumberType> m(this->rows, this->columns);
		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				m.at(r + 1, c + 1) = this->contents[r][c] * n;
			}
		}
		return m;
	}

	template<typename _InMatrixType>
	matrix<_NumberType> operator*(matrix<_InMatrixType>& n) {
		try {
			if (this->columns != n.rows) {
				throw matrix_exception(
					"Dot Multiplication Failed, Columns and Rows Not Equal",
					this->rows,
					this->columns
				);
			}

			matrix<_NumberType> m(this->rows, n.columns);

			_NumberType* n1 = new _NumberType[this->columns];
			_NumberType* n2 = new _NumberType[this->columns];

			for (unsigned short r = 0; r < this->rows; r++) {
				this->emplaceRow(r + 1, n1);
				for (unsigned short c = 0; c < n.columns; c++) {
					n.emplaceColumn(c + 1, n2);
					m.at(r + 1, c + 1) = evaluateMatrixNumberGrouping(n1, n2, this->columns);
				}
			}

			delete[] n1, n2;

			return m;
		}
		catch (matrix_exception& e) {
			e.printException();
		}
	}

	template<typename _InNumberType>
	matrix<_NumberType> operator/(const _InNumberType& n) {
		matrix<_NumberType> m(this->rows, this->columns);
		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				m.at(r + 1, c + 1) = this->contents[r][c] / n;
			}
		}
		return m;
	}

	template<typename _InMatrixType>
	matrix<_NumberType> operator+(const matrix<_InMatrixType>& n) {
		try {
			if (n.rows != this->rows) {
				throw matrix_exception(
					"Attempted Matrix Addition Not Possible\nRow Count Not Same", 
					this->rows, 
					this->columns
				);
			}
			else if (n.columns != this->columns) {
				throw matrix_exception(
					"Attempted Matrix Addition Not Possible\nColumn Count Not Same",
					this->rows,
					this->columns
				);
			}
			else {
				matrix<_NumberType> m(this->rows, this->columns);
				for (unsigned short r = 0; r < this->rows; r++) {
					for (unsigned short c = 0; c < this->columns; c++) {
						m.at(r + 1, c + 1) = this->contents[r][c] + n.contents[r][c];
					}
				}
				return m;
			}
		}
		catch (matrix_exception& e) {
			e.printException();
		}
	}

	template<typename _InMatrixType>
	matrix<_NumberType> operator-(const matrix<_InMatrixType>& n) {
		try {
			if (n.rows != this->rows) {
				throw matrix_exception(
					"Attempted Matrix Addition Not Possible\nRow Count Not Same",
					this->rows,
					this->columns
				);
			}
			else if (n.columns != this->columns) {
				throw matrix_exception(
					"Attempted Matrix Addition Not Possible\nColumn Count Not Same",
					this->rows,
					this->columns
				);
			}
			else {
				matrix<_NumberType> m(this->rows, this->columns);
				for (unsigned short r = 0; r < this->rows; r++) {
					for (unsigned short c = 0; c < this->columns; c++) {
						m.at(r + 1, c + 1) = this->contents[r][c] - n.contents[r][c];
					}
				}
				return m;
			}
		}
		catch (matrix_exception& e) {
			e.printException();
		}
	}

	template<typename _InMatrixType>
	void operator=(matrix<_InMatrixType>& n) {
		this->clean();
		
		this->rows = n.rows;
		this->columns = n.columns;
		
		this->initializeContents(this->rows, this->columns);

		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				this->contents[r][c] = (_NumberType)n.contents[r][c];
			}
		}
	}

	// fun usage of 'NOT' bitwise operation to signify matrix inverse
	// I'm proud I thought of it tbh, I shouldn't be but I am
	void operator~() {
		this->inverse();
	}

	template<typename _InNumberType>
	void operator*=(const _InNumberType& n) {
		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				this->contents[r][c] = this->contents[r][c] * n;
			}
		}
	}

	template<typename _InNumberType>
	void operator/=(const _InNumberType& n) {
		for (unsigned short r = 0; r < this->rows; r++) {
			for (unsigned short c = 0; c < this->columns; c++) {
				this->contents[r][c] = this->contents[r][c] / n;
			}
		}
	}

	template<typename _InMatrixType>
	void operator*=(matrix<_InMatrixType>& n) {
		matrix<_NumberType> m = this->operator*(n);
		this->clean();
		this->contents = m.contents();
	}

	template<typename _InMatrixType>
	void operator+=(matrix<_InMatrixType>& n) {
		matrix<_NumberType> m = this->operator+(n);
		this->clean();
		this->contents = m.contents();
	}

	template<typename _InMatrixType>
	void operator-=(matrix<_InMatrixType>& n) {
		matrix<_NumberType> m = this->operator-(n);
		this->clean();
		this->contents = m.contents();
	}
};