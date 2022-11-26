class Matrix
{
    public:
    string sourceFileName = "";
    string matrixName = "";
    vector<string> columns;
    uint columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;

    bool blockify();
    Matrix();
    Matrix(string matrixName);
    bool load();
    void printMatrix();
    void GetDimension(string &line);
    void makeMatrixPermanent();
};