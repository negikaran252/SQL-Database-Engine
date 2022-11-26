#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */

bool is_number(string& s)
{
    auto it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool syntacticParsePARTHASHJOIN()
{
    logger.log("syntacticParsePARTHASHJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[1] != "<-" || tokenizedQuery[2] != "JOIN" || tokenizedQuery[3] != "USING" || tokenizedQuery[4] != "PARTHASH" || tokenizedQuery[7] != "ON" || tokenizedQuery[11] != "BUFFER")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = PARTHASHJOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    parsedQuery.bufferSize = tokenizedQuery[12];

    if(!is_number(parsedQuery.bufferSize))
    {
        cout << "SYNTAX ERROR" << endl;
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
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParsePARTHASHJOIN()
{
    logger.log("semanticParseNESTEDJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

vector<Table*> partition(string &table_name, string& column_name)
{
    int number_blocks = stoi(parsedQuery.bufferSize) - 1;
    Table* first_table = tableCatalogue.getTable(table_name);
    vector<Table*> tables1(number_blocks);

    for(int i = 0; i < number_blocks; i++)
        tables1[i] = new Table(table_name + "_" + to_string(i), first_table->columns);

    int first_column_index = first_table->getColumnIndex(column_name);

    Cursor first_cursor = first_table->getCursor();
    vector<int> row1 = first_cursor.getNext();

    while(!row1.empty())
    {
        int hash_value = row1[first_column_index] % number_blocks;
        tables1[hash_value]->writeRow(row1);
        row1 = first_cursor.getNext();
    }

    for(int i = 0; i < number_blocks; i++)
    {
        tables1[i]->blockify();
        tableCatalogue.insertTable(tables1[i]);
    }   
    return tables1;    
}

void execute_join(Table* table1, Table*table2, Table* result_table1)
{
    int first_column_index = table1->getColumnIndex(parsedQuery.joinFirstColumnName);
    int second_column_index = table2->getColumnIndex(parsedQuery.joinSecondColumnName);

    Cursor first_cursor = table1->getCursor();
    Cursor second_cursor = table2->getCursor();


    vector<int> row1 = first_cursor.getNext();
    vector<int> row2 = second_cursor.getNext();

    while(!row1.empty())
    {
        while(!row2.empty())
        {  
            if(row1[first_column_index] == row2[second_column_index])
            {
                vector<int> row;
                row.insert(row.end(), row1.begin(), row1.end());
                row.insert(row.end(), row2.begin(), row2.end());
                result_table1->writeRow<int>(row);
            }
            row2 = second_cursor.getNext();
        }
        row1 = first_cursor.getNext();
        second_cursor = table2->getCursor();
        row2 = second_cursor.getNext();
    }

    
}



void executePARTHASHJOIN()
{
    logger.log("executePARTHASHJOIN");

    int number_blocks = stoi(parsedQuery.bufferSize) - 1;
    Table* first_table = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table* second_table = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    if(first_table->rowCount == 0 || second_table->rowCount == 0)
    {
        cout << "Empty table" << endl;
        return;
    }

    vector<string>columns;

    for(int i = 0; i < first_table->columnCount; i++)
        columns.push_back(first_table->columns[i]);
    
    for(int i = 0; i < second_table->columnCount; i++)
        columns.push_back(second_table->columns[i]);

    vector<Table*> tables1 = partition(parsedQuery.joinFirstRelationName, parsedQuery.joinFirstColumnName);
    vector<Table*> tables2 = partition(parsedQuery.joinSecondRelationName, parsedQuery.joinSecondColumnName);


    Table* result_table = new Table(parsedQuery.joinResultRelationName, columns);

    for(int i = 0; i < tables1.size(); i++)
    {
        Table* first_table = tables1[i];
        Table* second_table = tables2[i];

        if(first_table->rowCount > 0 && second_table->rowCount > 0)
        {
            execute_join(first_table, second_table, result_table);
        }
    }


    result_table->blockify();
    tableCatalogue.insertTable(result_table);
   

    uint count = first_table->blockCount + second_table->blockCount + result_table->blockCount;
    for(int i = 0; i < number_blocks; i++)
    {
        count += tables1[i]->blockCount * tables2[i]->blockCount;
    }

    cout<<"Number of Blocks Access: "<<count<<endl;

    for(int i = 0; i < tables1.size(); i++)
    {
        tableCatalogue.deleteTable(tables1[i]->tableName);
        tableCatalogue.deleteTable(tables2[i]->tableName);
    }

    return;
}