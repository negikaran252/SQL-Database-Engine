#include "global.h"


void MatrixCatalogue::insertMatrix(Matrix* matrix)
{
    logger.log("MatrixCatalogue::~insertMatrix"); 
    // cout<<" "<<matrix->matrixName<<endl;
    this->matrices[matrix->matrixName] = matrix;
}


Matrix* MatrixCatalogue::getMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::getMatrix"); 
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}

bool MatrixCatalogue::isMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::isMatrix"); 
    if (this->matrices.count(matrixName))
        return true;
    return false;
}

