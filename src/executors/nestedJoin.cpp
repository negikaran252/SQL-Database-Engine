#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */


bool is_num(string& s)
{
    auto it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool syntacticParseNESTEDJOIN()
{
    logger.log("syntacticParseNESTEDJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[1] != "<-" || tokenizedQuery[3] != "USING" || tokenizedQuery[7] != "ON" || tokenizedQuery[11] != "BUFFER")
    {
        std::cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = NESTEDJOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    parsedQuery.bufferSize=tokenizedQuery[12];

    if(!is_num(parsedQuery.bufferSize))
    {
        cout<<"Buffer Size is Not a numeric value"<<endl;
        return false;
    }
    if(stoi(parsedQuery.bufferSize)<3)
    {
        cout<<"We need atleast 3 buffers"<<endl;
        return false;
    }

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        std::cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseNESTEDJOIN()
{
    logger.log("semanticParseNESTEDJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        std::cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        std::cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        std::cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}
bool nestedjoinCond(int x,int y)
{
    switch(parsedQuery.joinBinaryOperator){
        case EQUAL: return x == y; break;
        case NOT_EQUAL: return x != y; break;
        case LESS_THAN: return x < y; break;
        case GREATER_THAN: return x > y; break;
        case GEQ: return x >= y; break;
        case LEQ: return x <= y; break;
        default: std::cout<<"Incorrect Binary Operator"<<endl; return false;
    }

}

void executeNESTEDJOIN()
{
    logger.log("executeNESTEDJOIN");
    Table table1 = *(tableCatalogue.getTable(parsedQuery.joinFirstRelationName));
    Table table2 = *(tableCatalogue.getTable(parsedQuery.joinSecondRelationName));

    vector<string> columns;

    //If both tables are the same i.e. NESTEDJOIN a a, then names are indexed as a1 and a2
    if(table1.tableName == table2.tableName){
        parsedQuery.joinFirstRelationName += "1";
        parsedQuery.joinSecondRelationName += "2";
    }

    //Creating list of column names and updating the indexes of the column
    int FirstColIndex=-1,SecondColIndex=-1;
    for (int columnCounter = 0; columnCounter < table1.columnCount; columnCounter++)
    {
        string columnName = table1.columns[columnCounter];
        if(parsedQuery.joinFirstColumnName==columnName)
            FirstColIndex=columnCounter;
        // if (table2.isColumn(columnName))
        // {
        //     columnName = parsedQuery.joinFirstRelationName + "_" + columnName;
        // }
        columns.emplace_back(columnName);
    }

    for (int columnCounter = 0; columnCounter < table2.columnCount; columnCounter++)
    {
        string columnName = table2.columns[columnCounter];
        if(parsedQuery.joinSecondColumnName==columnName)
            SecondColIndex=columnCounter;
        // if (table1.isColumn(columnName))
        // {
        //     columnName = parsedQuery.joinSecondRelationName + "_" + columnName;
        // }
        columns.emplace_back(columnName);
    }

    Table *resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    Cursor cursor1 = table1.getCursor();
    Cursor cursor2 = table2.getCursor();

    vector<int> row1 = cursor1.getNext();
    vector<int> row2;
    vector<int> resultantRow;
    resultantRow.reserve(resultantTable->columnCount);

    // getting the metadata i.e. number of rows which are stored in buffer and number of rows in a single page.
    int bf=stoi(parsedQuery.bufferSize);
    int TotalRowsInBuffer=(table1.maxRowsPerBlock)*(bf-2);
    int numRowsInBlock=table2.maxRowsPerBlock;
    int blockAccess=0;

    while(!row1.empty())
    {
        //storing rows of FirstRelation in R of size BufferSize-2
        vector<vector<int>> Rvec;
        int counter=0;
        for(int i=0;i<TotalRowsInBuffer && !row1.empty();i++)
        {
            Rvec.push_back(row1);
            row1=cursor1.getNext();
            counter++;
            if(counter==table1.maxRowsPerBlock)
            {
                counter=0;
                blockAccess++;
            }
        }

        if(counter!=0)
            blockAccess++;

        // storing the rows of SecondRelation in S of size one Buffer.
        cursor2=table2.getCursor();
        row2=cursor2.getNext();

        while(!row2.empty())
        {
            vector<vector<int>> Svec;
            for(int i=0;i<numRowsInBlock && !row2.empty();i++)
            {
                Svec.push_back(row2);
                row2=cursor2.getNext();
            }
            blockAccess++;

            //performing the join
            for(auto r1:Rvec)
            {
                for(auto r2:Svec)
                {
                    if(nestedjoinCond(r1[FirstColIndex],r2[SecondColIndex]))
                    {
                        resultantRow = r1;
                        resultantRow.insert(resultantRow.end(), r2.begin(), r2.end());
                        resultantTable->writeRow<int>(resultantRow);
                    }
                }
            }
        }
    }

    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
    cout<<"Column Count: "<<resultantTable->columnCount<<endl;
    cout<<"Row COunt: "<<resultantTable->rowCount<<endl;
    cout<<endl;
    cout<<"Number Of Block Access For Reading-> "<<blockAccess<<endl;
    cout<<"Number Of Block Access For Writing-> "<<resultantTable->blockCount<<endl;
    cout<<"Total Number Of Block Access-> "<<blockAccess+resultantTable->blockCount<<endl;


    return;
}