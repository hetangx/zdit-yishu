#ifndef DATAPROC_H
#define DATAPROC_H

struct RETURNJSONSTRUCT{
    const char* sid;
    const char* tid;
    const char* tm;
    const char* ref_id;
    int mode; /*mode: 1 -> ub, 2~7 -> wave, 8-> voiceprint*/
};

typedef struct RETURNJSONSTRUCT makeJsonSt;

#include "json.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "dataSqlite.h"
extern sqlite3* db;

#define JSONLENGTH 2000
#define JSON1ITEM 30



double quality[7];  

const char* parse1Helper_str(Value* v, char* key);
void makeJsonFront(char temp[], int cnum, makeJsonSt* mjst);
void makeJsonpower(double powquality, char **cptr);
void setHelper(Value* v, char* key, double item);
void CalcTHDua(char* payload, char** cptr);
void CalcTHDub(char* payload, char** cptr);
void CalcTHDuc(char* payload, char** cptr);
void CalcTHDia(char* payload, char** cptr);
void CalcTHDib(char* payload, char** cptr);
void CalcTHDic(char* payload, char** cptr);
void Calcpowquality(char* payload, char** cptr);
void CalcpowqualityV2(sqlite3 *db, char *substationid, char *transformerid, char **cptr);

void CutoffBuffer1(char* buffer);
double ParseWaveHelper(Value* v, int n);


char GetFigure(char* topicName);
void MessageSwitchQuality(char* topicName, char* payload, char** cptr);
void CalcUnbalance(char* payload, char** cptr);

#endif // !DATAPROC_H