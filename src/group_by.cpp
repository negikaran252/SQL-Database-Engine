#include "global.h"
bool syntacticParseGROUP()
{
    if(tokenizedQuery.size() != 9 || tokenizedQuery[1] != "<-"  || tokenizedQuery[2] != "GROUP" || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUPBY;
    parsedQuery.GBFinalRelation = tokenizedQuery[0];
    parsedQuery.group_byColumnName = tokenizedQuery[4];
    parsedQuery.GBOriginalRelation = tokenizedQuery[6];
    parsedQuery.groupOperator_name = tokenizedQuery[8].substr(0,3);

    if(parsedQuery.groupOperator_name != "MAX" && parsedQuery.groupOperator_name != "MIN" && parsedQuery.groupOperator_name != "SUM" && parsedQuery.groupOperator_name != "AVG")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    string temp = tokenizedQuery[8];
    if(temp.size() < 6)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if(temp[3] != '(' || temp[temp.length()-1] != ')')
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    
    int l = tokenizedQuery[8].length();
    parsedQuery.aggregate_columnName = tokenizedQuery[8].substr(4,l-5);
    return true;
}


bool semanticParseGROUP()
{
    // Checking Existence of Result Relation
    if(tableCatalogue.isTable(parsedQuery.GBFinalRelation))
    {
        cout << "SEMANTIC ERROR: Relation " << parsedQuery.GBFinalRelation << " Already exist" << endl;
        return false;
    }
    // Checking Existence of Relation on which operation to Perform
    if(!tableCatalogue.isTable(parsedQuery.GBOriginalRelation))
    {
        cout << "SEMANTIC ERROR: Relation " << parsedQuery.GBOriginalRelation << " does not exist" << endl;
        return false;
    }
    // Checking Existence of Column on which operations to Perform

    Table* table = tableCatalogue.getTable(parsedQuery.GBOriginalRelation);

    if(!table->isColumn(parsedQuery.group_byColumnName) || !table->isColumn(parsedQuery.aggregate_columnName))
    {
        cout << "SEMANTIC ERROR: Column does not exist in Relation " << endl;
        return false;
    }
    return true;
}


void executeGROUP()
{
    string relationName = parsedQuery.GBOriginalRelation;

    Table table1 = *(tableCatalogue.getTable(relationName));

    Cursor cursor = table1.getCursor();
    string col1 = parsedQuery.group_byColumnName;
    string col2 = parsedQuery.aggregate_columnName;
    string col3 = parsedQuery.groupOperator_name + parsedQuery.aggregate_columnName;

    vector<string>col_vect = {col1,col3};
    Table* result_table = new Table(parsedQuery.GBFinalRelation,col_vect);


    // find index of both columns and make a map by reading row by row

    int index1 = table1.getColumnIndex(col1);
    int index2 = table1.getColumnIndex(col2);

   

    map<int,int>mp; // map of <col1, col2>
    map<int,int>counter; // map of <col1, count> 

    // Traverse row by row

    vector<int> row1 = cursor.getNext();
    while(!row1.empty())
    {
        int key = row1[index1];
        int value = row1[index2];
        if(mp.find(key) == mp.end())
        {
            mp[key] = value;
            counter[key]++;
        }
        else
        {
            if(parsedQuery.groupOperator_name == "MAX")
            {
                mp[key] = max(mp[key],value);
            }
            else if(parsedQuery.groupOperator_name == "MIN")
            {
                mp[key] = min(mp[key],value);
            }
            else if(parsedQuery.groupOperator_name == "SUM")
            {
                mp[key] += value;
            }
            else if(parsedQuery.groupOperator_name == "AVG")
            {
                
                counter[key]++;
                mp[key] += value;
            }
            
        }
        row1 = cursor.getNext();
    }

    for(auto m:mp)
    {
        vector<int>row;
        row.push_back(m.first);
        if(parsedQuery.groupOperator_name == "AVG")
        {
            int ans = m.second/counter[m.first];
            row.push_back(ans);
        }
        else
        {
            row.push_back(m.second);
        }
        result_table->writeRow<int>(row);
    }
    result_table->blockify();
    tableCatalogue.insertTable(result_table);
    cout<<"Result Table Created"<<endl;

    cout<<table1.blockCount + result_table->blockCount<<" blocks used"<<endl;

    return;
}