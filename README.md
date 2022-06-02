# 通过百度API检测人脸特征，获取颜值、年龄、性别--C语言实现

## 一、创建应用获取AK、SK

用到的开源库有：[cJSON](https://blog.csdn.net/qq_45698138/article/details/125074927?spm=1001.2014.3001.5502)、[libcurl](https://blog.csdn.net/qq_45698138/article/details/125089144?spm=1001.2014.3001.5502)

百度API获取方式：https://cloud.baidu.com/

## 1.注册登陆后点击：人脸识别云服务

![image-20220602102448369](\img\1.png)

## 2.立即使用

![image-20220602102636313](\img\2.png)

## 3.免费尝鲜领取后、创建应用

![image-20220602102758488](\img\3.png)

## 4.拿到API Key 和Secret Key

![image-20220602102947774](\img\4.png)

# 二、通过API获取图片的颜值、年龄、性别信息

## 1.获取access_token

参考：[API文档](https://ai.baidu.com/ai-doc/FACE/yk37c1u4t)

C代码如下 ：用到的库有curl、cJSON

test.c

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>	//stat文件属性
#include <unistd.h>
#include "cJSON.h"
#include "base64.h"
#include "curl/curl.h"
const char *pic_dir = "./xx.jpg";		//图片的路径
const char *request_url = "https://aip.baidubce.com/rest/2.0/face/v3/detect";//post请求的url 不需要修改
const char *access_token_url =  "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials";		  //获取access_token的url 不需要修改
const char *api_key = "xxxxxx";			//自己的API Key 
const char *secret_key = "xxxxxxxx";	//自己的 Secret Key

//声明一个结构体存数据
struct MemoryStruct {
  char *memory;
  size_t size;
};
//获取网站传回的数据    
static size_t MyCallBack_WriteToken(void *contents, size_t size, size_t nmemb, void *userp)
{       
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize; 
}

static int get_access_token__(struct MemoryStruct *access_token) {
    CURL *curl;
    CURLcode result_code;
    int error_code = 0;
    int bufSize = strlen(access_token_url)+strlen(api_key)+strlen(secret_key)+50;
    char *url = (char *)malloc(sizeof(char)*bufSize);
    sprintf(url,"%s&client_id=%s&client_secret=%s",access_token_url,api_key,secret_key);//设置URL
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        //设置传输的数据
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)access_token);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MyCallBack_WriteToken);
        result_code = curl_easy_perform(curl);//执行 
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return 1;
        }
        curl_easy_cleanup(curl);
        error_code = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = 1;
    }
    return error_code;
}

void get_access_tokenStr(char **str)
{
    struct MemoryStruct *access_token;
    access_token = malloc(sizeof(struct MemoryStruct));
    access_token->memory = malloc(1);
    access_token->size = 0;

    get_access_token__(access_token);
    // printf("%s\n",access_token->memory);

    cJSON *root = cJSON_Parse(access_token->memory);
    cJSON *item = cJSON_GetObjectItem(root,"access_token");

    *str = (char *)malloc(strlen(item->valuestring) + 1);

    memcpy(*str,item->valuestring,strlen(item->valuestring) + 1);

    // char *s = cJSON_Print(root);
    // printf("%s\n",s);
    // free(s);
    free(access_token->memory);
    free(access_token);
}

/*test*/
void main(void)
{
    char *access_token;
    get_access_tokenStr(&access_token);
    printf("%s\n",access_token);
    free(access_token);
    return;
}

```

编译运行：

```shell
gcc test.c -I ./include -L ./lib -lcjson -lm
```

## 2.对图片进行base64编码

查看API文档请求格式为`application/json`,图片需要进行Base64编码、图片的格式可以为PNG、JGP、JPEG、BMP

![image-20220602105009681](\img\5.png)

①先在目录中保存一张包含人脸的图片例如1.png

![2](\img\6.png)

②对图片进行Base64编码

[base64编码](https://blog.csdn.net/qq_45698138/article/details/125101541?csdn_share_tail=%7B%22type%22%3A%22blog%22%2C%22rType%22%3A%22article%22%2C%22rId%22%3A%22125101541%22%2C%22source%22%3A%22qq_45698138%22%7D&ctrtid=D8weZ)

## 3.Post 请求，获取图片的颜值、年龄、性别

①封装请求头

请求格式如下：

![image-20220602153317880](\img\7.png)



```c


// curl发送http请求调用的回调函数，回调函数中对返回的存储在了结构体faceContet中
static size_t callback__(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize; 

}
// 人脸检测与属性分析
static int post_request(struct MemoryStruct *faceContet,const char *request_param,const char *access_token)
{
    char *url;
    int bufSize = strlen(request_url) + strlen(access_token) + 20;
    url = (char *)malloc(sizeof(char)*bufSize);
    // https://aip.baidubce.com/rest/2.0/face/v3/detect?access_token=【调用鉴权接口获取的token】
    sprintf(url,"%s?access_token=%s",request_url,access_token);

    CURL *curl;
    curl = curl_easy_init();
    if(curl)
    {
        CURLcode res;
        struct curl_header *header;

        //设置URL
        curl_easy_setopt(curl,CURLOPT_URL,url);
        //设置协议头
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers,"Content-Type:application/json;charset=UTF-8");

        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
        // "{"image":"027d8308a2ec665acb1bdf63e513bcb9","image_type":"FACE_TOKEN","face_field":"faceshape,facetype"}"
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,request_param);
        
        //设置post的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)faceContet);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,callback__);


        //设置传输的数据
        curl_easy_setopt(curl,CURLOPT_POST,1);
        //执行 
        curl_easy_perform(curl);

    }

}
//需要传入access_token
char * get_face_detch(const char * access_token)  //获取post请求返回的参数
{
    struct MemoryStruct *faceContet;
    faceContet = malloc(sizeof(struct MemoryStruct));
    faceContet->memory = malloc(1);
    faceContet->size = 0;
    char *request_param,*jpgBuf,*base64JpgBuf;
    //获取图片的base64编码作为http post请求的参数
    struct stat statBuf;
    stat(pic_dir,&statBuf);     //获取文件信息
    int fileSize = statBuf.st_size; //获取文件大小
    jpgBuf = malloc(sizeof(char)*fileSize + 1);
    FILE *file;
    file = fopen(pic_dir,"r");
    if(file == NULL)
    {
        perror("fopen");
        return NULL;
    }
    if(fread(jpgBuf,fileSize,1,file) == 0)
    {
        printf("读取文件失败！");
        // getchar();
        exit(0);
    }
    int size = (int)((float)fileSize*1.5);
    
    base64JpgBuf = (char *)malloc(size*fileSize*sizeof(char));
    base64_encode_file(jpgBuf,base64JpgBuf,fileSize);

    request_param = (char *)malloc(strlen(base64JpgBuf) + 100);
    //请求参数设置
    sprintf(request_param,"{\"image\":\"%s\",\"image_type\":\"BASE64\",\"face_field\":\"age,gender,beauty\"}",base64JpgBuf);

    post_request(faceContet,request_param,access_token);


    free(jpgBuf);
    free(base64JpgBuf);
    free(request_param);

    /*test*/
    // cJSON *root = cJSON_Parse(faceContet->memory);
    // char *s = cJSON_Print(root);
    // printf("%s\n",s);
    // free(s);
    // printf("%s\n",faceContet->memory);
    return faceContet->memory;
}

```

## 4.解析JSON数据

使用[cJSON解析](https://blog.csdn.net/qq_45698138/article/details/125074927?spm=1001.2014.3001.5502)

```c

typedef struct PersonalInfo
{
    int age;
    char gender[GENDER_BUF_SIZE];
    float beauty; 
}DataType;

void parse_face_info(char *faceJsonStr)
{
    DataType data;
    cJSON *root,*result,*face_list,*obj,*val; 
    root = cJSON_Parse(faceJsonStr);
    int flag = 0;
    if(root != NULL && root->type == cJSON_Object)
    {
        result = cJSON_GetObjectItem(root,"result"); 
        if(result != NULL && result->type == cJSON_Object)
        {
            face_list = cJSON_GetObjectItem(result,"face_list");
            if(face_list != NULL && face_list->type == cJSON_Array)
            {
                obj = cJSON_GetArrayItem(face_list,0);
                if(obj != NULL && obj->type == cJSON_Object)
                {
                    val = cJSON_GetObjectItem(obj,"age");
                    data.age = val->valueint;
                    val = cJSON_GetObjectItem(obj,"beauty");
                    data.beauty = val->valuedouble;
                    val = cJSON_GetObjectItem(obj,"gender");
                    if(val != NULL && val->type == cJSON_Object)
                    {
                        val = cJSON_GetObjectItem(val,"type");
                        strcpy(data.gender,val->valuestring);
                        flag = 1;
                    }
                }
            }
        }
    }
    if(flag)
        printf("%d\t%s\t%.2f\t\n",data.age,data.gender,data.beauty);
    else
    {
        printf("解析失败！\n"); 
        char *s = cJSON_Print(root);
        printf("%s\n",s);
        free(s);
    }
    
    cJSON_Delete(root);
}
```

最后效果如下：

![image-20220602185134423](\img\8.png)

**颜值挺高80.23！**



全部代码如下：

total.c

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cJSON.h"
#include "curl/curl.h"
#define GENDER_BUF_SIZE 10

// size_t 是一些C/C++标准在stddef.h中定义的，size_t 类型表示C中任何对象所能达到的最大长度，它是无符号整数。

//声明一个结构体
struct MemoryStruct {
  char *memory;
  size_t size;
};

typedef struct PersonalInfo
{
    int age;
    char gender[GENDER_BUF_SIZE];
    float beauty; 
}DataType;

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *pic_dir = "./1.png";		//图片路径

const char *request_url = "https://aip.baidubce.com/rest/2.0/face/v3/detect";
const char *access_token_url =  "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials";
const char *api_key = "xxxxx";			//自己的AK
const char *secret_key = "xxxx";		//自己的SK


char *base64_encode_file(const unsigned char * bindata, char * base64, int binlength)
{
    int i, j;
    unsigned char current;
 
    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];
 
        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];
 
        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];
 
        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return 0;
}

//获取网站传回的数据    
static size_t MyCallBack_WriteToken(void *contents, size_t size, size_t nmemb, void *userp)
{       
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize; 
}

static int get_access_token__(struct MemoryStruct *access_token) {
    CURL *curl;
    CURLcode result_code;
    int error_code = 0;
    int bufSize = strlen(access_token_url)+strlen(api_key)+strlen(secret_key)+50;
    char *url = (char *)malloc(sizeof(char)*bufSize);
    sprintf(url,"%s&client_id=%s&client_secret=%s",access_token_url,api_key,secret_key);//设置URL
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        //设置传输的数据
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)access_token);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MyCallBack_WriteToken);
        result_code = curl_easy_perform(curl);//执行 
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return 1;
        }
        curl_easy_cleanup(curl);
        error_code = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = 1;
    }
    return error_code;
}

void get_access_tokenStr(char **str)
{
    struct MemoryStruct *access_token;
    access_token = malloc(sizeof(struct MemoryStruct));
    access_token->memory = malloc(1);
    access_token->size = 0;

    get_access_token__(access_token);
    // printf("%s\n",access_token->memory);

    cJSON *root = cJSON_Parse(access_token->memory);
    cJSON *item = cJSON_GetObjectItem(root,"access_token");

    *str = (char *)malloc(strlen(item->valuestring) + 1);

    memcpy(*str,item->valuestring,strlen(item->valuestring) + 1);

    // char *s = cJSON_Print(root);
    // printf("%s\n",s);
    // free(s);
    free(access_token->memory);
    free(access_token);
}

// curl发送http请求调用的回调函数，回调函数中对返回的json格式的body进行了解析，解析结果储存在全局的静态变量当中
static size_t callback__(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize; 

}
// 人脸检测与属性分析
static int post_request(struct MemoryStruct *faceContet,const char *request_param,const char *access_token)
{
    char *url;
    int bufSize = strlen(request_url) + strlen(access_token) + 20;
    url = (char *)malloc(sizeof(char)*bufSize);
    // https://aip.baidubce.com/rest/2.0/face/v3/detect?access_token=【调用鉴权接口获取的token】
    sprintf(url,"%s?access_token=%s",request_url,access_token);

    CURL *curl;
    int error_code = 0;
    CURLcode result_code;
    curl = curl_easy_init();
    if(curl)
    {
        //设置URL
        curl_easy_setopt(curl,CURLOPT_URL,url);
        //设置协议头
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers,"Content-Type:application/json;charset=UTF-8");

        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
        // "{"image":"027d8308a2ec665acb1bdf63e513bcb9","image_type":"FACE_TOKEN","face_field":"faceshape,facetype"}"
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,request_param);
        
        //设置post的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)faceContet);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,callback__);


        //设置传输的数据
        curl_easy_setopt(curl,CURLOPT_POST,1);

        result_code = curl_easy_perform(curl);//执行 
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return 1;
        }
        curl_easy_cleanup(curl);
        error_code = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = 1;
    }
}

char * get_face_detch(const char * access_token)  //获取post请求返回的参数
{
    struct MemoryStruct *faceContet;
    faceContet = malloc(sizeof(struct MemoryStruct));
    faceContet->memory = malloc(1);
    faceContet->size = 0;
    char *request_param,*jpgBuf,*base64JpgBuf;
    //获取图片的base64编码作为http post请求的参数
    struct stat statBuf;
    stat(pic_dir,&statBuf);     //获取文件信息
    int fileSize = statBuf.st_size; //获取文件大小
    jpgBuf = malloc(sizeof(char)*fileSize + 1);
    FILE *file;
    file = fopen(pic_dir,"r");
    if(file == NULL)
    {
        perror("fopen");
        return NULL;
    }

    if(fread(jpgBuf,fileSize,1,file) == 0)
    {
        printf("读取文件失败！");
        // getchar();
        exit(0);
    }
    int size = (int)((float)fileSize*1.5);

    base64JpgBuf = (char *)malloc(size*sizeof(char));
    if(base64JpgBuf == NULL)
    {
        perror("malloc error!");
        return NULL;
    }
    bzero(base64JpgBuf,size);

    base64_encode_file(jpgBuf,base64JpgBuf,fileSize);

    request_param = (char *)malloc(size + 100);
    sprintf(request_param,"{\"image\":\"%s\",\"image_type\":\"BASE64\",\"face_field\":\"age,gender,beauty\"}",base64JpgBuf);

    post_request(faceContet,request_param,access_token);

    free(jpgBuf);
    free(base64JpgBuf);
    free(request_param);
    fclose(file);
    /*test*/
    // cJSON *root = cJSON_Parse(faceContet->memory);
    // char *s = cJSON_Print(root);
    // printf("%s\n",s);
    // free(s);
    // printf("%s\n",faceContet->memory);
    return faceContet->memory;
}


void parse_face_info(char *faceJsonStr)
{
    DataType data;
    cJSON *root,*result,*face_list,*obj,*val; 
    root = cJSON_Parse(faceJsonStr);
    int flag = 0;
    if(root != NULL && root->type == cJSON_Object)
    {
        result = cJSON_GetObjectItem(root,"result"); 
        if(result != NULL && result->type == cJSON_Object)
        {
            face_list = cJSON_GetObjectItem(result,"face_list");
            if(face_list != NULL && face_list->type == cJSON_Array)
            {
                obj = cJSON_GetArrayItem(face_list,0);
                if(obj != NULL && obj->type == cJSON_Object)
                {
                    val = cJSON_GetObjectItem(obj,"age");
                    data.age = val->valueint;
                    val = cJSON_GetObjectItem(obj,"beauty");
                    data.beauty = val->valuedouble;
                    val = cJSON_GetObjectItem(obj,"gender");
                    if(val != NULL && val->type == cJSON_Object)
                    {
                        val = cJSON_GetObjectItem(val,"type");
                        strcpy(data.gender,val->valuestring);
                        flag = 1;
                    }
                }
            }
        }
    }
    if(flag)
        printf("%d\t%s\t%.2f\t\n",data.age,data.gender,data.beauty);
    else
    {
        printf("解析失败！\n"); 
        char *s = cJSON_Print(root);
        printf("%s\n",s);
        free(s);
    }
    
    cJSON_Delete(root);
}

int main(int argc, char const *argv[])
{
    char *access_token,*faceJsonStr;
    curl_global_init(CURL_GLOBAL_ALL);
    // get_access_tokenStr(&access_token);	//只需要一个月获取一次
    access_token = "xxxxxxxx";				//自己的access_token
    // printf("%s\n",access_token);
    faceJsonStr = get_face_detch(access_token);
    parse_face_info(faceJsonStr);

    free(faceJsonStr);
    return 0;
}

// 参考：
// https://curl.se/libcurl/c/example.html
```

编译运行

```shell
gcc total.c -I ./include -L ./lib -lcurl -lcjson -lm
./a.out
```



后面结合v4l2摄像头测试......

待更新......
