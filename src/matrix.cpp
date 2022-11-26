#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

void Matrix::GetDimension(string &line) // To Confirm - Is there any need for Handling for 0 element in a row ?
{
    int count = 0;
    for (int i = 0; i < line.length(); i++)
    {
        if (line[i] == ',')
            count++;
    }
    count++;
    this->columnCount = count;
    this->rowCount = count;
    this->blockCount = ceil(count / (MATRIX_PAGE_DIMENSION * 1.0));
}

/**
 * @brief Function to check if page already exists
 * 
 */
bool isPageExists(string pageName)
{
    string fileName = "../data/temp/" + pageName + ".txt";
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}


void preprocess_row(string &line, vector<int>&data_row)
{
    // Push_back each number in the row to the block_row vector
    stringstream ss(line);
    string token;
    while (getline(ss, token, ','))
    {
        data_row.push_back(stoi(token));
    }
}



/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Table::blockify");
    ifstream fin1(this->sourceFileName, ios::in);

    if (fin1.peek() == ifstream::traits_type::eof()) {
        cout << "OPERATIONAL ERROR: File is empty." << endl;
        return false;
    }
    // Reading first Line for storing the Meta Data
    string line1;
    getline(fin1, line1);
    this->GetDimension(line1);
    fin1.close();



    ifstream fin(this->sourceFileName, ios::in);

   // Task -1 Set the number of rows in the file
    for(int page_row = 0; page_row < this->blockCount; page_row++)
    {
        for(int row_counter=page_row*MATRIX_PAGE_DIMENSION;row_counter<MATRIX_PAGE_DIMENSION*(page_row+1);row_counter++)
        {
            string line;
            getline(fin, line);
            vector<int>data_row;
            if(row_counter>=this->rowCount)
                data_row=vector<int>(columnCount,0);
            else
                preprocess_row(line, data_row);
            
            // Adding Padding to the data_row vector
            int data_row_sz = MATRIX_PAGE_DIMENSION*this->blockCount;
            for(int i=data_row.size();i< data_row_sz;i++)
                data_row.push_back(0);

            for(int page_col=0;page_col<this->blockCount;page_col++)
            {
                string curr_page_name= "../data/temp/" + this->matrixName + "_" + to_string(page_row) + "_" + to_string(page_col) + ".txt";
                if(isPageExists(curr_page_name))
                {
                    // open curr_page_name in append mode
                    ofstream fout(curr_page_name, ios::app);
                    for(int i = page_col*MATRIX_PAGE_DIMENSION; i < (page_col+1)*MATRIX_PAGE_DIMENSION; i++)
                    {
                        fout << data_row[i] << " ";
                        fout << endl;
                    }
                    
                    fout.close();
                }
                else
                {
                    // open curr_page_name in write mode
                    ofstream fout(curr_page_name, ios::app);
                    for(int i = page_col*MATRIX_PAGE_DIMENSION; i < (page_col+1)*MATRIX_PAGE_DIMENSION; i++)
                    {
                        fout << data_row[i] << " ";
                    }
                    fout << endl;
                    fout.close();
                }
                
            }

        }
    }

    fin.close();
    
   

    
    return true;
}



bool Matrix::load()
{
    logger.log("Matrix::load");
    if (this->blockify())
        return true;
    return false;
}




void Matrix::printMatrix()
{
    logger.log("Matrix::print"); 
    

    string matrix_addr = "../data/temp/" + (parsedQuery.printMatrixName) + "_0_0.txt";
    ifstream matrix_file(matrix_addr);
    string line;
    // Read the file line by line
    uint col_count = 20;
    int row_count = 0, column_count = min(col_count, this->columnCount);
    while (getline(matrix_file, line) && row_count < 20)
    {
       
        // Store the line in a vector
        vector<int> line_vector;
        stringstream ss(line);
        string token;
        while (getline(ss, token, ' '))
        {
            line_vector.push_back(stoi(token));
        }
        for (int i = 0; i < column_count; i++)
        {
            cout << line_vector[i] << " ";
        }
        cout << endl;
        row_count++;
    }

    cout<<row_count<<" Rows and "<<column_count<<" Columns are Printed"<<endl;

}


string preprocess_row_export(string & row)
{
    vector<int>row_vec;
    stringstream ss(row);
    string token;
    while (getline(ss, token, ' '))
    {
        row_vec.push_back(stoi(token));
    }
    string final_row = "";
    for(int i = 0; i < MATRIX_PAGE_DIMENSION; i++)
    {
        if(i == MATRIX_PAGE_DIMENSION - 1)
            final_row += to_string(row_vec[i]);
        else
            final_row += to_string(row_vec[i]) + ",";
    }
    return final_row;
}


string remove_padding(string & complete_row, int matrix_col)
{
    vector<int>row_vec;
    stringstream ss(complete_row);
    string token;
    while (getline(ss, token, ','))
    {
        row_vec.push_back(stoi(token));
    }
    string final_row = "";
    for(int i = 0; i < matrix_col; i++)
    {
        if(i == matrix_col - 1)
            final_row += to_string(row_vec[i]);
        else
            final_row += to_string(row_vec[i]) + ",";
    }
    return final_row;
}


void Matrix::makeMatrixPermanent()
{
    // To delete the already existing matrix file
    string matrix_addr = "../data/" + (parsedQuery.exportMatrixName) + ".csv";
    
    ofstream matrix_file(matrix_addr, ios::out);
    
    int MatrowCount = this->rowCount;
    for(int page_row = 0; page_row < this->blockCount; page_row++)
    {
        for(int row_counter=0;row_counter<MATRIX_PAGE_DIMENSION;row_counter++)
        {
            
            string complete_row_line = "";
            for(int page_col=0;page_col<this->blockCount;page_col++)
            {
                string curr_page_name= "../data/temp/" + this->matrixName + "_" + to_string(page_row) + "_" + to_string(page_col) + ".txt";
                ifstream fout(curr_page_name);
                
                int temp_row_count = 0;
                string line;
                while(temp_row_count < row_counter)
                {                    
                    getline(fout, line);
                    temp_row_count++;
                }
                getline(fout, line);
  
                line = preprocess_row_export(line);

                if(page_col==0)
                    complete_row_line += line;
                else
                    complete_row_line += "," + line;
              
                
                fout.close();
            }

            complete_row_line = remove_padding(complete_row_line, this->columnCount);
            matrix_file << complete_row_line << endl;
            MatrowCount -= 1;

            if(MatrowCount == 0)
                break;
        }
    }
    matrix_file.close();

}