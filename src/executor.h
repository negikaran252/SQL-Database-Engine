#include"semanticParser.h"

void executeCommand();

void executeCLEAR();
void executeCROSS();
void executeDISTINCT();
void executeEXPORT();
void executeEXPORTMATRIX();
void executeINDEX();
void executeJOIN();
void executeLIST();
void executeLOAD();
void executeLOADMATRIX();
void executePRINT();
void executePRINTMATRIX();
void executePROJECTION();
void executeRENAME();
void executeSELECTION();
void executeSORT();
void executeGROUP();
void executeSOURCE();
void executeCROSS_TRANSPOSE_MATRIX();
void executePARTHASHJOIN();
void executeNESTEDJOIN();

bool evaluateBinOp(int value1, int value2, BinaryOperator binaryOperator);
void printRowCount(int rowCount);