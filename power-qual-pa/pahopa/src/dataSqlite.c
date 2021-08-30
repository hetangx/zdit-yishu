#include "dataSqlite.h"

char *tableItems[8] = {
    "TRANSFORMERID", "UB", "THDUA", "THDUB", "THDUC", "THDIA", "THDIB", "THDIC"};

/*向数据库中插入值*/
int DbInsertValue(sqlite3 *db, char *substationid, char *transformerid, char *item, double value)
{
    int rc;
    if ((rc = DbIfExist(db, substationid)) == 1)
    {
        if ((rc = DbRecordIfExist(db, substationid, transformerid)) == 1)
        {
            DbUpdateRecord(db, substationid, transformerid, item, value);
        }
        else if (rc == 0)
        {
            DbInsertRecord(db, substationid, transformerid, item, value);
        }
    }
    else if (rc == 0)
    {
        DbCreateTable(db, substationid);
        DbInsertRecord(db, substationid, transformerid, item, value);
    }
    return rc;
}

/*sqlite DB回调函数，显示返回的键值对*/
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
/*sqlite DB回调函数，传入一个指针, 指向double数组，用于保存返回值*/
int callbackGetData(void *dataArray, int argc, char **argv, char **azColName)
{
    int i;
    double *result = dataArray;
    for (i = 0; i < argc; i++)
    {
        if (argv[i])
        {
            *result = (double)atof(argv[i]);
        }
        else
        {
            *result = 0;
        }
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        result++;
    }
    printf("\n");
    return 0;
}

/*sqlite DB回调函数，打印所有表的名字*/
int callbackShowAllTable(void *outMessage, int argc, char **argv, char **ColName)
{
    // 1. 显示表名
    if (argc == 1)
    {
        printf("%s\n", argv[0]);
    }
    else
    {
        printf("SELECT tables name wrong, find code error\n");
        return -1;
    }
    return 0;
}

/*sqlite DB回调函数，传入int 变量作为返回值，
1 - 存在
0 - 不存在*/
int callbackIfTableExist(void *outMessage, int argc, char **argv, char **ColName)
{
    *(int *)outMessage = (int)atoi(argv[0]);
    return 0;
}

/*OpenDB, 打开数据库, 用法:
    sqlite3* db;
    char* filename = "dbname.db";
    OpenDB(&db, filename);
* 结果会打印在屏幕中
*/
void DbOpen(sqlite3 **db, char *filename)
{
    int rc;
    rc = sqlite3_open(filename, db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
}

/*显示表中所有名字*/
void DbTables(sqlite3 *db)
{
    int rc;
    char *zErrMsg = 0;
    char *sqlAllTableName = " SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";

    rc = sqlite3_exec(db, sqlAllTableName, callbackShowAllTable, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Table showed successfully\n");
    }
}

/*查询数据库中是否存在某张表，db为指向sqlite3的指针, substationid为变电站ID, 返回int
1 - 存在
0 - 不存在
-1 - 程序运行错误*/
int DbIfExist(sqlite3 *db, char *substationid)
{
    int rc;
    char *zErrMsg = 0;
    char sqlIfExist[1024];
    sprintf(sqlIfExist, "select count(*) as c from sqlite_master where type ='table' and name ='SID%s'; ", substationid);

    int ifExist = -1;
    rc = sqlite3_exec(db, sqlIfExist, callbackIfTableExist, &ifExist, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (ifExist == 1)
    {
        printf("Table %s exist\n", substationid);
        // 插入项
    }
    else
    {
        printf("table %s null\n", substationid);
        // 创建表
    }
    return ifExist;
}

/*创建表*/
void DbCreateTable(sqlite3 *db, char *substationid)
{
    int rc;
    char *zErrMsg = 0;

    char sqlCreateTable[1024];
    sprintf(sqlCreateTable, "CREATE TABLE SID%s(TRANSFORMERID TEXT PRIMARY KEY NOT NULL,UB REAL,THDUA REAL,THDUB REAL,THDUC REAL,THDIA REAL,THDIB REAL,THDIC REAL);", substationid);
    rc = sqlite3_exec(db, sqlCreateTable, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Table created successfully\n");
    }
}

/*项是否存在, 传入变电站ID和变压器ID, 返回int
1 - 存在
0 - 不存在
-1 - 出错*/
int DbRecordIfExist(sqlite3 *db, char *substationid, char *transformerid)
{
    int rc;
    char *zErrMsg = 0;
    char sqlRecordIfExist[1024];
    sprintf(sqlRecordIfExist, "select count(*) from SID%s where TRANSFORMERID='%s';", substationid, transformerid);

    int ifTableExist = -1;
    rc = sqlite3_exec(db, sqlRecordIfExist, callbackIfTableExist, &ifTableExist, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    if (ifTableExist == 1)
    {
        printf("Record %s exist\n", transformerid);
    }
    else
    {
        printf("table %s null\n", transformerid);
    }
    return ifTableExist;
}

// 一项纪录里的内容

/*若记录不存在, 插入记录
DbInsertRecord(db, "2", "AQ", tableItems[2], 1.5);
char* tableItems[8] = {
"TRANSFORMERID","UB","THDUA","THDUB","THDUC","THDIA","THDIB","THDIC"
};
*/
void DbInsertRecord(sqlite3 *db, char *substationid, char *transformerid, char *item, double valueToInsert)
{

    int rc;
    char *zErrMsg = 0;

    char sqlInsertRecord[1024];
    sprintf(sqlInsertRecord, "INSERT INTO SID%s(TRANSFORMERID,%s) VALUES (\"%s\", %.4f);", substationid, item, transformerid, valueToInsert);
    rc = sqlite3_exec(db, sqlInsertRecord, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Insert record successfully\n");
    }
}

/*若记录存在, 更新单个数据
DbUpdateRecord(db, "2", "A", "UB", 2.7);
*/
void DbUpdateRecord(sqlite3 *db, char *substationid, char *transformerid, char *item, double valueToInsert)
{
    int rc;
    char *zErrMsg = 0;
    char sqlUpdateRecord[1024];

    // UPDATE SID2 SET THDUA=2.5 WHERE TRANSFORMERID="B";
    sprintf(sqlUpdateRecord, "UPDATE SID%s SET %s=%.4f WHERE TRANSFORMERID=\"%s\";", substationid, item, valueToInsert, transformerid);
    rc = sqlite3_exec(db, sqlUpdateRecord, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Update record successfully\n");
    }
}

/*得到全部值
 double result[8];
 DbGetAllData(db, "2", "A", result);
    SELECT * FROM SID2 WHERE TRANSFORMERID="B";
*/
void DbGetAllData(sqlite3 *db, char *substationid, char *transformerid, double allData[])
{
    int rc;
    char *zErrMsg = 0;
    char sqlGetAllData[1024];

    sprintf(sqlGetAllData, "SELECT * FROM SID%s WHERE TRANSFORMERID=\"%s\";", substationid, transformerid);
    rc = sqlite3_exec(db, sqlGetAllData, callbackGetData, (void *)allData, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Insert single record successfully\n");
    }
}