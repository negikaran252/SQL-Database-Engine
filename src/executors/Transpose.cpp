#include "global.h"


bool syntacticParseCROSS_TRANSPOSE_MATRIX(){
    logger.log("syntacticParseCROSS_TRANSPOSE_MATRIX");
    if(tokenizedQuery.size()!= 3){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    parsedQuery.queryType = CROSS_TRANSPOSE_MATRIX;
    parsedQuery.Matrix_One = tokenizedQuery[1];
    parsedQuery.Matrix_Two = tokenizedQuery[2];
    return true;
}

bool semanticParseCROSS_TRANSPOSE_MATRIX()
{
    logger.log("semanticParseCROSS_TRANSPOSE_MATRIX");
    if (!matrixCatalogue.isMatrix(parsedQuery.Matrix_One))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    if (!matrixCatalogue.isMatrix(parsedQuery.Matrix_Two))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void transpose2DArrays(vector<vector<int>> &arr1,vector<vector<int>> &arr2)
{
    logger.log("Transpose::transpose2DArrays");
    for(int i=0;i<arr1.size();i++)
    {
        for(int j=0;j<arr1[0].size();j++)
        {
            swap(arr1[i][j],arr2[j][i]);
        }
    }
}

void writeBackIntoPageBlock(vector<vector<int>> &arr,string page_name)
{
    logger.log("Transpose::writeBackIntoPageBlock");
    ofstream fout(page_name, ios::out);
    for (int i = 0; i < arr.size(); i++)
    {
        for (int j = 0; j < arr[0].size(); j++)
        {
            if (j != 0)
                fout << " ";
            fout << arr[i][j];
        }
        fout << endl;
    }
    fout.close();
}

void executeCROSS_TRANSPOSE_MATRIX()
{
    // Transpose Code
    logger.log("Transpose::executeCROSS_TRANSPOSE_MATRIX");
    Matrix* matrix1=matrixCatalogue.getMatrix(parsedQuery.Matrix_One);
    Matrix* matrix2=matrixCatalogue.getMatrix(parsedQuery.Matrix_Two);
    

    // getting matrices metadata
    string matrix_one_name=matrix1->matrixName;
    string matrix_two_name=matrix2->matrixName;

    uint num_blocks=matrix1->blockCount;

    // load pages
    for(int i=0;i<num_blocks;i++)
    {
        for(int j=0;j<num_blocks;j++)
        {
            string page_name_one="../data/temp/"+matrix_one_name+"_"+to_string(i)+"_"+to_string(j)+".txt";
            string page_name_two="../data/temp/"+matrix_two_name+"_"+to_string(j)+"_"+to_string(i)+".txt";
            
            vector<vector<int>> arr1(MATRIX_PAGE_DIMENSION,vector<int>(MATRIX_PAGE_DIMENSION,0));
            vector<vector<int>> arr2(MATRIX_PAGE_DIMENSION,vector<int>(MATRIX_PAGE_DIMENSION,0));

            ifstream fin1(page_name_one, ios::in);
            for(int bi=0;bi<MATRIX_PAGE_DIMENSION;bi++)
            {
                string line;
                getline(fin1, line);
                stringstream ss(line);
                string token;
                vector<int> row_vec;
                while (getline(ss, token, ' '))
                {           
                    row_vec.push_back(stoi(token));
                }
                for(int bj=0;bj<row_vec.size();bj++)
                {
                    arr1[bi][bj]=row_vec[bj];
                }
            }
            fin1.close();

            ifstream fin2(page_name_two, ios::in);
            for(int bi=0;bi<MATRIX_PAGE_DIMENSION;bi++)
            {
                string line;
                getline(fin2, line);
                stringstream ss(line);
                string token;
                vector<int> row_vec;
                while (getline(ss, token, ' '))
                {           
                    row_vec.push_back(stoi(token));
                }
                for(int bj=0;bj<row_vec.size();bj++)
                {
                    arr2[bi][bj]=row_vec[bj];
                }
            }
            fin2.close();
            transpose2DArrays(arr1,arr2);
            writeBackIntoPageBlock(arr1,page_name_one);
            writeBackIntoPageBlock(arr2,page_name_two);
        }
    }


}