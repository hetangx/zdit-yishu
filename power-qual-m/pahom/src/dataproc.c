#include "dataproc.h"

complex x[WAVELENGTH], y[WAVELENGTH], * W; /*输入序列,变换核*/
complex* W;
double result[WAVERESULT];

/*set unbalance new ImbNgA*/
void makeJsonUb(double newImbNgA, char** cptr)
{
    Allocator* A = NewAllocator();
    Value* v = NewValue(A);
    bool ret = Parse(v, *cptr);
    if (ret == true)
    {
        setHelper(v, "ImbNgA", newImbNgA);
        const char* str = Stringify(v);
        strcpy(*cptr, str);
        // strcpy_s(*cptr, JSONLENGTH, str);
    }
    ReleaseAllocator(A);
}

/*数据读取和处理*/
void data()
{
    for (int s = 0; s < 128; s++)   //找出一个周期
    {
        if (((y[s].real * y[s + 1].real) < 0) & (y[s].real < 0))
        {
            for (int k = 0; k < 78; k++)
            {
                x[k].real = y[k + s].real;
                //printf("%f\n", x[k].real);
            }
            //printf("%d\n", s);
            break;
        }
    }
}

/*fft计算*/
void fft()
{
    int i = 0, j = 0, k = 0, l = 0;
    complex up, down, product;

    change();  //调用变址函数
    for (int i = 0; i < 78; i++)
    {
        printf("%f\n", x[i].real);
    }
    for (i = 0; i < log(128) / log(2); i++)        /*一级蝶形运算 stage */
    {
        l = 1 << i;
        for (j = 0; j < 128; j += 2 * l)     /*一组蝶形运算 group,每个group的蝶形因子乘数不同*/
        {
            for (k = 0; k < l; k++)        /*一个蝶形运算 每个group内的蝶形运算*/
            {
                mul(x[j + k + l], W[128 * k / 2 / l], &product);
                add(x[j + k], product, &up);
                sub(x[j + k], product, &down);
                x[j + k] = up;
                x[j + k + l] = down;
            }
        }
    }
}

/*变址计算，将x(n)码位倒置*/
void change()
{
    complex temp;
    unsigned short i = 0, j = 0, k = 0;
    double t;
    for (i = 0; i < 128; i++)
    {
        k = i; j = 0;
        t = (log(128) / log(2));
        while ((t--) > 0)    //利用按位与以及循环实现码位颠倒
        {
            j = j << 1;
            j |= (k & 1);
            k = k >> 1;
        }
        if (j > i)    //将x(n)的码位互换
        {
            temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }
}

/*初始化变换核，定义一个变换核，相当于旋转因子WAP*/
void initW()
{
    double PI;
    PI = atan(1) * 4;
    W = (complex*)malloc(sizeof(complex) * 128);  //生成变换核
    if (W)
    {
        for (int i = 0; i < 128; i++)
        {
            W[i].real = cos(2 * PI / 128 * i);   //用欧拉公式计算旋转因子
            W[i].img = -1 * sin(2 * PI / 128 * i);
        }
    }

}

/*输出傅里叶变换的结果*/
void output()
{
    int i;
    double sum = 0;
    double thd;
    double temp;

    //printf("The result are as follows：\n");
    for (i = 0; i < 32; i++)
    {
        int a = (int)(log(128) / log(2)) - 1;
        temp = pow((x[i].real * x[i].real) / pow(2, 2 * (double)a) + (x[i].img * x[i].img) / pow(2, 2 * (double)a), 0.5);
        result[i] = temp;  //32个谐波值保存在result数组前32位中
        //printf("%f\n", result[i]);
    }

    for (i = 2; i < 32; i++)
    {
        sum += pow(x[i].real, 2);
        thd = pow(sum / (x[1].real * x[1].real), 0.5);
    }
    result[32] = thd;  //将thd的值保存到result数组的最后一位

    //输出测试//
    //for (i = 0; i < 33; i++)
    //{
    //    printf("%f\n", result[i]);
    //}
    //printf("THD=%f", thd);
}

void add(complex a, complex b, complex* c)  //复数加法的定义
{
    c->real = a.real + b.real;
    c->img = a.img + b.img;
}


void mul(complex a, complex b, complex* c)  //复数乘法的定义
{
    c->real = a.real * b.real - a.img * b.img;
    c->img = a.real * b.img + a.img * b.real;
}


void sub(complex a, complex b, complex* c)  //复数减法的定义
{
    c->real = a.real - b.real;
    c->img = a.img - b.img;
}


/*接收到的payload尾部带有乱码, 影响json库解析, 可用此函数截断'}'后面的所有内容*/
void CutoffBuffer(char* buffer)
{
    //printf("length buffer: %d\n", (int)strlen(buffer));
    char* ptr = NULL;
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
void setHelper(Value* v, char* key, double item)
{
    Value* temp = ObjGet(v, key);
    if (temp == 0)
    {
        return;
    }
    SetDouble(temp, item);
}

/*拼接json母体字符串*/
void makeJsonFront(char temp[], int cnum, makeJsonSt* mjst)
{
    // un balance
    if (mjst->mode == 1)
    {
        const char* mod1 = "{\"substationid\":";
        //TEMPSID 
        const char* mod2 = ",\"transformerid\":";
        //TEMPTID
        const char* mod3 = ",\"time\":\"";
        //TEMPTIME
        const char* mod4 = "\",\"ImbNgA\":0.5}";
        //TEMPIMBGA

        sprintf(temp, "%s%s%s%s%s%s%s", mod1, mjst->sid, mod2, mjst->tid, mod3, mjst->tm, mod4);
        // strncpy_s(temp, cnum, mod1, strlen(mod1));
        // strncat_s(temp, cnum, mjst->sid, strlen(mjst->sid));
        // strncat_s(temp, cnum, mod2, strlen(mod2));
        // strncat_s(temp, cnum, mjst->tid, strlen(mjst->tid));
        // strncat_s(temp, cnum, mod3, strlen(mod3));
        // strncat_s(temp, cnum, mjst->tm, strlen(mjst->tm));
        // strncat_s(temp, cnum, mod4, strlen(mod4));
        return;
    }

    // voice print
    if (mjst->mode == 8) 
    {
        /*
        const char* mod1 = "{\"substationid\":";
        //TEMPSID 
        const char* mod2 = ",\"transformerid\":";
        //TEMPTID
        const char* mod3 = ",\"time\":\"";
        //TEMPTIME
        const char* mod4 = "\",\"powerlity\":0.5}";
        //TEMPIMBGA

        sprintf(temp, "%s%s%s%s%s%s%s", mod1, mjst->sid, mod2, mjst->tid, mod3, mjst->tm, mod4);
        // strncpy_s(temp, cnum, mod1, strlen(mod1));
        // strncat_s(temp, cnum, mjst->sid, strlen(mjst->sid));
        // strncat_s(temp, cnum, mod2, strlen(mod2));
        // strncat_s(temp, cnum, mjst->tid, strlen(mjst->tid));
        // strncat_s(temp, cnum, mod3, strlen(mod3));
        // strncat_s(temp, cnum, mjst->tm, strlen(mjst->tm));
        // strncat_s(temp, cnum, mod4, strlen(mod4));
        return;
        */
    }

    // mjst.mode = 2~7
    {
        const char* mod1 = "{\"substationid\":";
        //TEMPSID 
        const char* mod2 = ",\"transformerid\":";
        //TEMPTID
        const char* mod3 = ",\"time\":\"";
        //TEMPTIME
        const char* mod4 = "\",\"ref_id\":";
        //TEMPRID
        const char* mod5 = NULL;
        if (mjst->mode == 2)
        {
            mod5 = ",\"HphV0_phsA\":1.1234,\"HphV1_phsA\":16.0000,\"HphV2_phsA\":1.050,\"HphV3_phsA\":13.000,\"HphV4_phsA\":14.012,\"HphV5_phsA\":1.333,\"HphV6_phsA\":1.222,\"HphV7_phsA\":1.124,\"HphV8_phsA\":2.0000,\"HphV9_phsA\":1.050,\"HphV10_phsA\":13.000,\"HphV11_phsA\":1.1234,\"HphV12_phsA\":20.0000,\"HphV13_phsA\":1.050,\"HphV14_phsA\":13.000,\"HphV15_phsA\":14.012,\"HphV16_phsA\":12.333,\"HphV17_phsA\":15.222,\"HphV18_phsA\":16,\"HphV19_phsA\":20.1000,\"HphV20_phsA\":1.050,\"HphV21_phsA\":13.000,\"HphV22_phsA\":1.1234,\"HphV23_phsA\":220.0000,\"HphV24_phsA\":1.050,\"HphV25_phsA\":13.000,\"HphV26_phsA\":14.012,\"HphV27_phsA\":12.333,\"HphV28_phsA\":15.222,\"HphV29_phsA\":0.8,\"HphV30_phsA\":2.0000,\"HphV31_phsA\":1.050,\"THDUa\":0.5}";
        }
        if (mjst->mode == 3)
        {
            mod5 = ",\"HphV0_phsB\":1.1234,\"HphV1_phsB\":16.0000,\"HphV2_phsB\":1.050,\"HphV3_phsB\":13.000,\"HphV4_phsB\":14.012,\"HphV5_phsB\":1.333,\"HphV6_phsB\":1.222,\"HphV7_phsB\":1.124,\"HphV8_phsB\":2.0000,\"HphV9_phsB\":1.050,\"HphV10_phsB\":13.000,\"HphV11_phsB\":1.1234,\"HphV12_phsB\":20.0000,\"HphV13_phsB\":1.050,\"HphV14_phsB\":13.000,\"HphV15_phsB\":14.012,\"HphV16_phsB\":12.333,\"HphV17_phsB\":15.222,\"HphV18_phsB\":16,\"HphV19_phsB\":20.1000,\"HphV20_phsB\":1.050,\"HphV21_phsB\":13.000,\"HphV22_phsB\":1.1234,\"HphV23_phsB\":220.0000,\"HphV24_phsB\":1.050,\"HphV25_phsB\":13.000,\"HphV26_phsB\":14.012,\"HphV27_phsB\":12.333,\"HphV28_phsB\":15.222,\"HphV29_phsB\":0.8,\"HphV30_phsB\":2.0000,\"HphV31_phsB\":1.050,\"THDUb\":0.5}";
        }
        if (mjst->mode == 4)
        {
            mod5 = ",\"HphV0_phsC\":1.1234,\"HphV1_phsC\":16.0000,\"HphV2_phsC\":1.050,\"HphV3_phsC\":13.000,\"HphV4_phsC\":14.012,\"HphV5_phsC\":1.333,\"HphV6_phsC\":1.222,\"HphV7_phsC\":1.124,\"HphV8_phsC\":2.0000,\"HphV9_phsC\":1.050,\"HphV10_phsC\":13.000,\"HphV11_phsC\":1.1234,\"HphV12_phsC\":20.0000,\"HphV13_phsC\":1.050,\"HphV14_phsC\":13.000,\"HphV15_phsC\":14.012,\"HphV16_phsC\":12.333,\"HphV17_phsC\":15.222,\"HphV18_phsC\":16,\"HphV19_phsC\":20.1000,\"HphV20_phsC\":1.050,\"HphV21_phsC\":13.000,\"HphV22_phsC\":1.1234,\"HphV23_phsC\":220.0000,\"HphV24_phsC\":1.050,\"HphV25_phsC\":13.000,\"HphV26_phsC\":14.012,\"HphV27_phsC\":12.333,\"HphV28_phsC\":15.222,\"HphV29_phsC\":0.8,\"HphV30_phsC\":2.0000,\"HphV31_phsC\":1.050,\"THDUc\":0.5}";
        }
        if (mjst->mode == 5)
        {
            mod5 = ",\"HA0_phsA\":1.1234,\"HA1_phsA\":16.0000,\"HA2_phsA\":1.050,\"HA3_phsA\":13.000,\"HA4_phsA\":14.012,\"HA5_phsA\":1.333,\"HA6_phsA\":1.222,\"HA7_phsA\":1.124,\"HA8_phsA\":2.0000,\"HA9_phsA\":1.050,\"HA10_phsA\":13.000,\"HA11_phsA\":1.1234,\"HA12_phsA\":20.0000,\"HA13_phsA\":1.050,\"HA14_phsA\":13.000,\"HA15_phsA\":14.012,\"HA16_phsA\":12.333,\"HA17_phsA\":15.222,\"HA18_phsA\":16,\"HA19_phsA\":20.1000,\"HA20_phsA\":1.050,\"HA21_phsA\":13.000,\"HA22_phsA\":1.1234,\"HA23_phsA\":220.0000,\"HA24_phsA\":1.050,\"HA25_phsA\":13.000,\"HA26_phsA\":14.012,\"HA27_phsA\":12.333,\"HA28_phsA\":15.222,\"HA29_phsA\":0.8,\"HA30_phsA\":2.0000,\"HA31_phsA\":1.050,\"THDIa\":0.5}";
        }
        if (mjst->mode == 6)
        {
            mod5 = ",\"HA0_phsB\":1.1234,\"HA1_phsB\":16.0000,\"HA2_phsB\":1.050,\"HA3_phsB\":13.000,\"HA4_phsB\":14.012,\"HA5_phsB\":1.333,\"HA6_phsB\":1.222,\"HA7_phsB\":1.124,\"HA8_phsB\":2.0000,\"HA9_phsB\":1.050,\"HA10_phsB\":13.000,\"HA11_phsB\":1.1234,\"HA12_phsB\":20.0000,\"HA13_phsB\":1.050,\"HA14_phsB\":13.000,\"HA15_phsB\":14.012,\"HA16_phsB\":12.333,\"HA17_phsB\":15.222,\"HA18_phsB\":16,\"HA19_phsB\":20.1000,\"HA20_phsB\":1.050,\"HA21_phsB\":13.000,\"HA22_phsB\":1.1234,\"HA23_phsB\":220.0000,\"HA24_phsB\":1.050,\"HA25_phsB\":13.000,\"HA26_phsB\":14.012,\"HA27_phsB\":12.333,\"HA28_phsB\":15.222,\"HA29_phsB\":0.8,\"HA30_phsB\":2.0000,\"HA31_phsB\":1.050,\"THDIb\":0.5}";
        }
        if (mjst->mode == 7)
        {
            mod5 = ",\"HA0_phsC\":1.1234,\"HA1_phsC\":16.0000,\"HA2_phsC\":1.050,\"HA3_phsC\":13.000,\"HA4_phsC\":14.012,\"HA5_phsC\":1.333,\"HA6_phsC\":1.222,\"HA7_phsC\":1.124,\"HA8_phsC\":2.0000,\"HA9_phsC\":1.050,\"HA10_phsC\":13.000,\"HA11_phsC\":1.1234,\"HA12_phsC\":20.0000,\"HA13_phsC\":1.050,\"HA14_phsC\":13.000,\"HA15_phsC\":14.012,\"HA16_phsC\":12.333,\"HA17_phsC\":15.222,\"HA18_phsC\":16,\"HA19_phsC\":20.1000,\"HA20_phsC\":1.050,\"HA21_phsC\":13.000,\"HA22_phsC\":1.1234,\"HA23_phsC\":220.0000,\"HA24_phsC\":1.050,\"HA25_phsC\":13.000,\"HA26_phsC\":14.012,\"HA27_phsC\":12.333,\"HA28_phsC\":15.222,\"HA29_phsC\":0.8,\"HA30_phsC\":2.0000,\"HA31_phsC\":1.050,\"THDIc\":0.5}";
        }
        if (mod5 == NULL)
        {
            return;
        }
        sprintf(temp, "%s%s%s%s%s%s%s%s%s", mod1, mjst->sid,mod2, mjst->tid,mod3,mjst->tm, mod4,  mjst->ref_id,mod5);
        // strncpy_s(temp, cnum, mod1, strlen(mod1));
        // strncat_s(temp, cnum, mjst->sid, strlen(mjst->sid));
        // strncat_s(temp, cnum, mod2, strlen(mod2));
        // strncat_s(temp, cnum, mjst->tid, strlen(mjst->tid));
        // strncat_s(temp, cnum, mod3, strlen(mod3));
        // strncat_s(temp, cnum, mjst->tm, strlen(mjst->tm));
        // strncat_s(temp, cnum, mod4, strlen(mod4));
        // strncat_s(temp, cnum, mjst->ref_id, strlen(mjst->ref_id));
        // strncat_s(temp, cnum, mod5, strlen(mod5));
    }
}

void makeJsonWaveBack(int fig, double newWave[], int wavenum, char** cptr)
{
    Allocator* A = NewAllocator();
    Value* v = NewValue(A);
    bool ret = Parse(v, *cptr);
    if (ret == true)
    {
        if (fig == 2)
        {
            char* keys[] = {
                "HphV0_phsA", "HphV1_phsA", "HphV2_phsA","HphV3_phsA","HphV4_phsA","HphV5_phsA","HphV6_phsA","HphV7_phsA",
                "HphV8_phsA","HphV9_phsA","HphV10_phsA","HphV11_phsA","HphV12_phsA","HphV13_phsA","HphV14_phsA","HphV15_phsA",
                "HphV16_phsA","HphV17_phsA","HphV18_phsA","HphV19_phsA","HphV20_phsA","HphV21_phsA","HphV22_phsA","HphV23_phsA",
                "HphV24_phsA","HphV25_phsA","HphV26_phsA","HphV27_phsA","HphV28_phsA","HphV29_phsA","HphV30_phsA","HphV31_phsA",
                "THDUa"
            };
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
        }
        if (fig == 3)
        {
            char* keys[] = {
                "HphV0_phsB", "HphV1_phsB", "HphV2_phsB","HphV3_phsB","HphV4_phsB","HphV5_phsB","HphV6_phsB","HphV7_phsB",
                "HphV8_phsB","HphV9_phsB","HphV10_phsB","HphV11_phsB","HphV12_phsB","HphV13_phsB","HphV14_phsB","HphV15_phsB",
                "HphV16_phsB","HphV17_phsB","HphV18_phsB","HphV19_phsB","HphV20_phsB","HphV21_phsB","HphV22_phsB","HphV23_phsB",
                "HphV24_phsB","HphV25_phsB","HphV26_phsB","HphV27_phsB","HphV28_phsB","HphV29_phsB","HphV30_phsB","HphV31_phsB",
                "THDUb"
            };
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
        }
		if (fig == 4)
		{
			char* keys[] = {
	            "HphV0_phsC", "HphV1_phsC", "HphV2_phsC","HphV3_phsC","HphV4_phsC","HphV5_phsC","HphV6_phsC","HphV7_phsC",
	            "HphV8_phsC","HphV9_phsC","HphV10_phsC","HphV11_phsC","HphV12_phsC","HphV13_phsC","HphV14_phsC","HphV15_phsC",
	            "HphV16_phsC","HphV17_phsC","HphV18_phsC","HphV19_phsC","HphV20_phsC","HphV21_phsC","HphV22_phsC","HphV23_phsC",
	            "HphV24_phsC","HphV25_phsC","HphV26_phsC","HphV27_phsC","HphV28_phsC","HphV29_phsC","HphV30_phsC","HphV31_phsC",
	            "THDUc"
			};
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
		}
        if (fig == 5)
        {
            char* keys[] = {
                "HA0_phsA", "HA1_phsA", "HA2_phsA","HA3_phsA","HA4_phsA","HA5_phsA","HA6_phsA","HA7_phsA",
                "HA8_phsA","HA9_phsA","HA10_phsA","HA11_phsA","HA12_phsA","HA13_phsA","HA14_phsA","HA15_phsA",
                "HA16_phsA","HA17_phsA","HA18_phsA","HA19_phsA","HA20_phsA","HA21_phsA","HA22_phsA","HA23_phsA",
                "HA24_phsA","HA25_phsA","HA26_phsA","HA27_phsA","HA28_phsA","HA29_phsA","HA30_phsA","HA31_phsA",
                "THDIa"
            };
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
        }
        if (fig == 6)
        {
            char* keys[] = {
                "HA0_phsB", "HA1_phsB", "HA2_phsB","HA3_phsB","HA4_phsB","HA5_phsB","HA6_phsB","HA7_phsB",
                "HA8_phsB","HA9_phsB","HA10_phsB","HA11_phsB","HA12_phsB","HA13_phsB","HA14_phsB","HA15_phsB",
                "HA16_phsB","HA17_phsB","HA18_phsB","HA19_phsB","HA20_phsB","HA21_phsB","HA22_phsB","HA23_phsB",
                "HA24_phsB","HA25_phsB","HA26_phsB","HA27_phsB","HA28_phsB","HA29_phsB","HA30_phsB","HA31_phsB",
                "THDIb"
            };
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
        }
        if (fig == 7)
        {
            char* keys[] = {
                "HA0_phsC", "HA1_phsC", "HA2_phsC","HA3_phsC","HA4_phsC","HA5_phsC","HA6_phsC","HA7_phsC",
                "HA8_phsC","HA9_phsC","HA10_phsC","HA11_phsC","HA12_phsC","HA13_phsC","HA14_phsC","HA15_phsC",
                "HA16_phsC","HA17_phsC","HA18_phsC","HA19_phsC","HA20_phsC","HA21_phsC","HA22_phsC","HA23_phsC",
                "HA24_phsC","HA25_phsC","HA26_phsC","HA27_phsC","HA28_phsC","HA29_phsC","HA30_phsC","HA31_phsC",
                "THDIc"
            };
            for (int i = 0; i < wavenum; i++)
            {
                setHelper(v, keys[i], newWave[i]);
            }
        }

    }
    const char* str = Stringify(v);
    strcpy(*cptr, str);
    // strcpy_s(*cptr, JSONLENGTH, str);
    ReleaseAllocator(A);
}

/*parse double from array by order*/
double ParseWaveHelper(Value* v, int n)
{
    Value* temp = ArrayGet(v, n);
    if (temp == 0)
    {
        return -1;
    }
    const double* t = GetDouble(temp);
    return *t;
}

/*parse double item by key*/
double Parse1Helper(Value* v, char* key)
{
    Value* temp = ObjGet(v, key);
    if (temp == 0)
    {
        return -1;
    }
    const double* t = GetDouble(temp);
    return *t;
}

/*parse string item by key*/
const char* parse1Helper_str(Value* v, char* key)
{
    Value* osid = ObjGet(v, key);
    if (osid == 0)
    {
        return "-1";
    }
    return Stringify(osid);
}

/*get topic figure from topic name*/
char GetFigure(char* topicName)
{
    char* fig = strstr(topicName, "/up");
    char a = 0;
    if (fig)
    {
        fig--;
        a = *fig;
        // printf("a: %c\n", a);
    }
    return a;
}

/*Switch fig*/
void MessageSwitch(char* topicName, char* payload, char** cptr)
{
    // need to reconstruct
    // use function pointer array to re-construct message switch
    char fig = GetFigure(topicName);
    switch (fig)
    {
    case '1':
        // un balance
        CalcUnbalance(payload, cptr);
        break;
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        // wave
        CalcWave(payload, (fig - '0'), cptr);
        break;
    case '8':
        //voiceprint;
        CalcVoice(payload, cptr);

        break;
    default:
        printf("get fig error, fig=%c\n", fig);
   
        break;
    }
    return;
}


/*计算三项不平衡*/
double unbalance(double out[])
{
    double avr = (out[4] + out[12] + out[20]) / 3.0;
    double max1 = fabs(avr - out[4]);
    double max2 = fabs(avr - out[12]);
    double max3 = fabs(avr - out[20]);

    double max4 = fmax(max1, max2);
    double max5 = fmax(max4, max3);

    double unblance = max5 / avr;

    return unblance;
}

void CalcUnbalance(char* payload, char** cptr)
{
    time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
    char otm[30];
    sprintf(otm, "%d-%02d-%02d %02d:%02d:%02d", (1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour+8,p->tm_min,p->tm_sec );
    
    Allocator* A = NewAllocator();
    Value* v = NewValue(A);
    // CutoffBuffer1(payload);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse unbalance payload error\n");
        return;
    }

    double out[JSON1ITEM]; 
    char* keys[JSON1ITEM] = { "substationid", "transformerid", "time",
            "Va", "Ia", "Fa", "Posa", "Nega", "VAa", "Elea", "Viba",
            "Vb", "Ib", "Fb", "Posb", "Negb", "VAb", "Eleb", "Vibb",
            "Vc", "Ic", "Fc", "Posc", "Negc", "VAc", "Elec", "Vibc",
            "Temp1", "Temp2", "Temp3" };

    for (int i = 3; i < 30; i++)
    {
        out[i] = Parse1Helper(v, keys[i]);
    }

    double ub = unbalance(out);
    const char* osid = parse1Helper_str(v, "substationid");
    const char* otid = parse1Helper_str(v, "transformerid");

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
    // strcpy_s(*cptr, JSONLENGTH, front);
    makeJsonUb(ub, cptr);

    // free memory
    ReleaseAllocator(A);
}



void CalcWave(char* payload, int fig, char** cptr)
{
    time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
    char otm[30];
    sprintf(otm, "%d-%02d-%02d %02d:%02d:%02d", (1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour+8,p->tm_min,p->tm_sec );
    
    Allocator* A = NewAllocator();
    Value* v = NewValue(A);
    // CutoffBuffer1(payload);
    bool ret = Parse(v, payload);
    if (ret != true)
    {
        printf("Parse wave payload error\n");
        return;
    }

    char* parsecontent[6] = {
        "Vwfa", "Vwfb", "Vwfc", "Cwfa", "Cwfb", "Cwfc"
    };

    Value* waveArray = ObjGet(v, parsecontent[fig-2]);
    if (waveArray == 0)
    {
        printf("parse wave json content: %s error\n", parsecontent[fig - 2]);
        return;
    }

    for (int i = 0; i < WAVELENGTH; i++)
    {
        double temp = ParseWaveHelper(waveArray, i);
        y[i].real = temp; //若报错, 试试y
    }

    data(); //调用数据读取和处理
    initW();//调用变换核
    fft();//调用快速傅里叶变换
    output();//调用输出傅里叶变换结果函数



    char* osid = (char*)parse1Helper_str(v, "substationid");
    char* otid = (char*)parse1Helper_str(v, "transformerid");
    char* orid = (char*)parse1Helper_str(v, "ref_id");

    char front[JSONLENGTH];
    makeJsonSt mjst;
    mjst.mode = fig;
    mjst.sid = osid;
    mjst.tid = otid;
    mjst.tm = otm;
    mjst.ref_id = orid;

    makeJsonFront(front, JSONLENGTH, &mjst);
    memset(*cptr, 0, JSONLENGTH);
    strcpy(*cptr, front);
    // strcpy_s(*cptr, JSONLENGTH, front);

    makeJsonWaveBack(fig, result, WAVERESULT, cptr);

    // free memory
    ReleaseAllocator(A);
    memset(result, 0, 33);
    memset(x, 0, sizeof(complex) * WAVELENGTH);
    memset(y, 0, sizeof(complex) * WAVELENGTH);
    memset(W, 0, sizeof(complex) * WAVELENGTH);
}

void CalcVoice(char* payload, char** cptr)
{
    printf("voiceprint need to be done\n");
}

int ShowVoiceRaw(char* filename)
{
    printf("need to be upgraded\n");
    return 1;
    // int err;
    // FILE* fp;
    // err = fopen_s(&fp, filename, "rb");
    // if (err == 0)
    // {
    //     printf("file: %s was opened\n", filename);
    // }
    // else
    // {
    //     printf("file: %s was not opened\n", filename);
    //     return -1;
    // }

    // if (fp == NULL)
    // {
    //     return -1;
    // }

    // fseek(fp, 0L, SEEK_END);
    // int length = ftell(fp);
    // printf("length: %d\n", length);
    // rewind(fp);

    // int a;
    // int range = length / 16;
    // for (int i = 0; i < range; i++)
    // {
    //     printf("%X\t", i);
    //     for (int j = 0; j < 16; j++)
    //     {
    //         a = fgetc(fp);
    //         printf("%02X ", a);
    //     }
    //     printf("\n");
    // }

    // fclose(fp);
    // return 1;
}


// mode = 1 -> 依大端序解析，F0*8+FF, mode = 0 -> 依小端序解析，F0+FF*8
int ParseVoiceRaw(char* filename, vac* v, int mode)
{
    printf("need to be upgraded\n");
    return 0;
    // FILE* fp;
    // fopen_s(&fp, filename, "rb");
    // if (fp == NULL)
    // {
    //     printf("open file error\n");
    //     return -1;
    // }

    // fseek(fp, 0L, SEEK_END);
    // int length = ftell(fp);
    // rewind(fp);

    // int a, b;
    // v->count = 0;
    // for (int i = 0; i < length; i++)
    // {
    //     a = fgetc(fp);
    //     i++;
    //     b = fgetc(fp);
    //     if (mode == 1)
    //     {
    //         v->voice[v->count] = a * 8 + b;
    //     }
    //     else if (mode == 0)
    //     {
    //         v->voice[v->count] = a + 8 * b;
    //     }
    //     v->count++;
    // }


    // fclose(fp);
    // return 0;
}
