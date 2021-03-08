#include "matrix.h"

matrix_exception::matrix_exception(const char* what, unsigned short rows, unsigned short columns, const char** info ){
	this->what = what;
	this->info = info;
	this->rows = rows;
	this->columns = columns;
}

void matrix_exception::printException(int exitCode) {
	printf("Matrix Exception Thown\nMatrix<%d by %d>\n", this->rows, this->columns);

	printf("WHAT: %s\n", this->what);

	if (this->info == NULL)
		return;
	
	unsigned int i = 0;
	while (this->info[i] != NULL)
		printf("%s", this->info[i++]);

	exit(exitCode);
}