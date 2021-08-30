#include "power.h"

extern char *tableItems[8];
//  {"TRANSFORMERID", "UB", "THDUA", "THDUB", "THDUC", "THDIA", "THDIB", "THDIC"};

// int DbInsertValue(sqlite3 *db, char *substationid, char *transformerid, char *item, double value);

void makeJsonpower(double powquality, char **cptr)
{
    Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, *cptr);
    if (ret == true)
    {
        setHelper(v, "powerlity", powquality);
        const char *str = Stringify(v);
        strcpy(*cptr, str);
    }
    else{
        printf("parse jsonpower error in data.c:8\n");
    }
    ReleaseAllocator(A);
}

void CutoffBuffer1(char *buffer)
{
    //printf("length buffer: %d\n", (int)strlen(buffer));
    char *ptr = NULL;
    char tofind = '}';
    ptr = strrchr(buffer, tofind);
    if (ptr)
    {
        ptr++;
        int len = (int)strlen(buffer);
        int plen = (int)strlen(ptr);
        //printf("content buffer: %s\ncontent ptr: %s\n", buffer, ptr);
        //printf("length buffer: %d\nlength ptr: %d\n", len, plen);

        buffer[len - plen] = 0;
    }
    //printf("work out: %s\n", buffer);
}

/*set double by key*/
void setHelper(Value *v, char *key, double item)
{
    Value *temp = ObjGet(v, key);
    if (temp == 0)
    {
        return;
    }
    SetDouble(temp, item);
}

void makeJsonFront(char temp[], int cnum, makeJsonSt *mjst)
{
    const char *mod1 = "{\"substationid\":\"";
    //TEMPSID
    const char *mod2 = "\",\"transformerid\":\"";
    //TEMPTID
    const char *mod3 = "\",\"time\":\"";
    //TEMPTIME
    const char *mod4 = "\",\"powerlity\":-1}";
    //TEMPIMBGA

    sprintf(temp, "%s%s%s%s%s%s%s", mod1, mjst->sid, mod2, mjst->tid, mod3, mjst->tm, mod4);
   // printf("makeJsonFront:\n%s\n", temp);
}

/*parse double from array by order*/
double ParseWaveHelper(Value *v, int n)
{
    Value *temp = ArrayGet(v, n);
    if (temp == 0)
    {
        return -1;
    }
    const double *t = GetDouble(temp);
    return *t;
}

/*parse double item by key*/
double Parse1Helper(Value *v, char *key)
{
    Value *temp = ObjGet(v, key);
    if (temp == 0)
    {
        return -1;
    }
    const double *t = GetDouble(temp);
    return *t;
}

/*parse string item by key*/
const char *parse1Helper_strV2(Value *v, char *key)
{
    Value *osid = ObjGet(v, key);
    if (osid == 0)
    {
        return "-1";
    }
    return GetStr(osid);
    // return Stringify(osid);
}

const char *parse1Helper_str(Value *v, char *key)
{
    Value *osid = ObjGet(v, key);
    if (osid == 0)
    {
        return "-1";
    }
    return Stringify(osid);
}

/*get topic figure from topic name*/
char GetFigure(char *topicName)
{
    char *fig = strstr(topicName, "/return");
    char a = 0;
    if (fig)
    {
        fig--;
        a = *fig;
        // printf("a: %c\n", a);
    }
    return a;
}

/*Switch Quality Message*/
void MessageSwitchQuality(char *topicName, char *payload, char **cptr)
{
    // need to reconstruct
    // use function pointer array to re-construct message switch
    char fig = GetFigure(topicName);
    switch (fig)
    {
    case '2':
        CalcTHDua(payload, cptr); //锟斤拷锟斤拷锟斤拷THDua锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[0]锟斤拷
        break;
    case '3':
        CalcTHDub(payload, cptr); //锟斤拷锟斤拷锟斤拷THDub锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[1]锟斤拷
        break;
    case '4':
        CalcTHDuc(payload, cptr); //锟斤拷锟斤拷锟斤拷THDuc锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[2]锟斤拷
        break;
    case '5':
        CalcTHDia(payload, cptr); //锟斤拷锟斤拷锟斤拷THDia锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[3]锟斤拷
        break;
    case '6':
        CalcTHDib(payload, cptr); //锟斤拷锟斤拷锟斤拷THDib锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[4]锟斤拷
        break;
    case '7':
        CalcTHDic(payload, cptr); //锟斤拷锟斤拷锟斤拷THDic锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[5]锟斤拷
        break;
    case '1':
        CalcUnbalance(payload, cptr); //锟斤拷锟斤拷锟斤拷unbalance锟斤拷值锟斤拷锟斤拷锟斤拷锟絨uality[6]锟斤拷
        break;

    default:
        printf("get fig error, fig=%c\n", fig);
        break;
    }
}

void CalcTHDua(char *payload, char **cptr)
{

    Allocator *A = NewAllocator();
    Value *v = NewValue(A);

    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDua payload error\n");
        return;
    }

    char *keys[3] = {
        "substationid", "transformerid", "THDUa"};
    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[2], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);

    ReleaseAllocator(A);
    // quality[0] = thd; //锟斤拷锟絋HDUa锟斤拷值
    // Calcpowquality(payload, cptr);
}
void CalcTHDub(char *payload, char **cptr)
{
    Allocator *A = NewAllocator();
    Value *v = NewValue(A);

    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDub payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "THDUb"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[3], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);

    ReleaseAllocator(A);
    // quality[1] = out2[35]; //锟斤拷锟絋HUDb锟斤拷值
    // Calcpowquality(payload, cptr);
}
void CalcTHDuc(char *payload, char **cptr)
{
    Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDuc payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "THDUc"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[4], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);

    ReleaseAllocator(A);
    // quality[2] = out3[35]; //锟斤拷锟絋HDUc锟斤拷值
    // Calcpowquality(payload, cptr);
}
void CalcTHDia(char *payload, char **cptr)
{
    Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDia payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "THDIa"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[5], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);
    
    ReleaseAllocator(A);

    // quality[3] = out4[35]; //锟斤拷锟絋HDIa锟斤拷值
    // Calcpowquality(payload, cptr);
}
void CalcTHDib(char *payload, char **cptr)
{

       Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDib payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "THDIb"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[6], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);
    
    ReleaseAllocator(A);

    // quality[4] = out5[35]; // 锟斤拷锟絋HDIb锟斤拷值
    // Calcpowquality(payload, cptr);
}
void CalcTHDic(char *payload, char **cptr)
{

       Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse THDic payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "THDIc"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double thd = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[7], thd);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);
    
    ReleaseAllocator(A);
}

void CalcUnbalance(char *payload, char **cptr)
{
    Allocator *A = NewAllocator();
    Value *v = NewValue(A);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse unbalance payload error\n");
        return;
    }

    char *keys[3] = {"substationid", "transformerid", "ImbNgA"};

    const char *sid = parse1Helper_strV2(v, keys[0]);
    const char *tid = parse1Helper_strV2(v, keys[1]);
    double ub = Parse1Helper(v, keys[2]);

    DbInsertValue(db, (char*)sid, (char*)tid, tableItems[1], ub);
    CalcpowqualityV2(db, (char*)sid, (char*)tid, cptr);
    
    ReleaseAllocator(A);
}

void CalcpowqualityV2(sqlite3 *db, char *substationid, char *transformerid, char **cptr)
{
    double values[7];
    double maxTHD, temp, powquality;
    double PI = atan(1) * 4;
    DbGetAllData(db, substationid, transformerid, values);
    //计算电能质量

    maxTHD = values[0];

    for (int j = 0; j < 6; j++)
        if (values[j] > maxTHD)
        {
            maxTHD = values[j];
        }
    temp = atan(maxTHD) / (PI / 2); //锟斤拷一锟斤拷

    //printf("%f\n", temp);

    powquality = 1 - (0.45 * temp + 0.55 * values[6]); //锟斤拷锟絋HD锟斤拷锟斤拷锟斤拷嗖黄斤拷锟饺ｏ拷锟斤拷权平锟斤拷
    //printf("%f", powquality);

    // 计算时间
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    char otm[30];
    sprintf(otm, "%d-%02d-%02d %02d:%02d:%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    char front[JSONLENGTH];
    makeJsonSt mjst;
    mjst.mode = 1;
    mjst.sid = substationid;
    mjst.tid = transformerid;
    mjst.tm = otm;
    mjst.ref_id = NULL;

    makeJsonFront(front, JSONLENGTH, &mjst);

    memset(*cptr, 0, JSONLENGTH);
    strcpy(*cptr, front);
    makeJsonpower(powquality, cptr);
}
void Calcpowquality(char *payload, char **cptr)
{
    double PI;
    PI = atan(1) * 4;
    float maxTHD, temp;
    float powquality;

    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    char otm[30];
    sprintf(otm, "%d-%02d-%02d %02d:%02d:%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    Allocator *A = NewAllocator();
    Value *v = NewValue(A);

    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse Calcpowquality payload error\n");
        return;
    }

    maxTHD = quality[0];

    for (int j = 0; j < 6; j++)
        if (quality[j] > maxTHD)
        {
            maxTHD = quality[j];
        } //锟揭筹拷锟斤拷锟街?

    temp = atan(maxTHD) / (PI / 2); //锟斤拷一锟斤拷

    //printf("%f\n", temp);

    powquality = 1 - (0.45 * temp + 0.55 * quality[6]); //锟斤拷锟絋HD锟斤拷锟斤拷锟斤拷嗖黄斤拷锟饺ｏ拷锟斤拷权平锟斤拷
    //printf("%f", powquality);

    const char *osid = parse1Helper_str(v, "substationid");
    const char *otid = parse1Helper_str(v, "transformerid");

    char front[JSONLENGTH];
    makeJsonSt mjst;
    mjst.mode = 1;
    mjst.sid = osid;
    mjst.tid = otid;
    mjst.tm = otm;
    mjst.ref_id = NULL;

    makeJsonFront(front, JSONLENGTH, &mjst);
    memset(*cptr, 0, JSONLENGTH);

    strcpy(*cptr, front);
    makeJsonpower(powquality, cptr);
    // free memory
    ReleaseAllocator(A);
}
