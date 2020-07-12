//#include <QCoreApplication>

#include "down_load_file.hpp"

//采用CURLOPT_RESUME_FROM_LARGE 实现文件断点续传功能

#include <sys/stat.h>

#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>

//thirty libs
#include <curl/curl.h>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "blockingconcurrentqueue.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//user header
#include "infor_unpacket.hpp"
#include "update_version.hpp"

/* 不支持 https 协议，需要编译 重新编译 */
#define ali_url "http://xxxx.oss-cn-hongkong.aliyuncs.com/abc.zip"
#define ali_urls "https://xxxxx.oss-cn-hongkong.aliyuncs.com/abc.zip"
#define file_path               "/userdata/"
#define time_out                30
#define try_times               3
#define download_timeout        500  //s
#define firmware_input      "firmwareApi"


static curl_off_t remain_file_length = 0;
static curl_off_t local_file_length = 0 ;
static int progress_count = 0;
static int wifi_connect_count = 0;

/**
 *
*/

bool getUrl(char *filename,char *url)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    if ( wifi_connect_count <= 1)
    {
        spdlog::info("wait for 15 second, internet stabilize ...\n");
        //std::this_thread::sleep_for(std::chrono::seconds(15));
        wifi_connect_count = 2;
    }else{
        spdlog::info("wait for 10 second, internet stabilize ...\n");
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        wifi_connect_count = 2;
    }
    if ((fp = fopen(filename, "w")) == NULL)  /// 返回结果用文件存储
    {
        perror("open file failur..");
        return false;
    }
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: Agent-007");
    curl = curl_easy_init();    // 初始化
    if (curl)
    {
        //curl_easy_setopt(curl, CURLOPT_PROXY, "10.99.60.201:8080");/// 代理
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);/// 改协议头
        curl_easy_setopt(curl, CURLOPT_URL,url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);      ///将返回的html主体数据输出到fp指向的文件
        //curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp);   ///将返回的http头输出到fp指向的文件
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, false); ///将返回的http头输出
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1);                ///> consol debug info
        res = curl_easy_perform(curl);                     /// 执行
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        fclose(fp);
        if (res != 0) {
            remove(filename);
            spdlog::info("get URL data download failure....,perform URLcode: {} \n",res);
            return false;
        }
        spdlog::info("get URL data download okay\n");
        return true;
    }
    fclose(fp);
    spdlog::info("init curl pointer failure...\n");
    return false;
}

struct MemoryStruct
{
    char *memory;
    size_t size;
};

static int hm_server_ad_fwrite(void *buffer, size_t size, size_t nmemb, void *userp)
{

#if 1
    size_t  realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        /** out of memory!*/
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), (char*)buffer,realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    spdlog::info("length:{} contents: {} ", mem->size, mem->memory);
    return realsize;
#endif

#if 0
    int block = size * nmemb;
    FILE *fp = NULL;
    //char *pTempName = "c:\\temp.json";
    char * pTempName = (char*) post_sava_path;

    if(userp == NULL)
        return -1;

    fp = fopen(pTempName,"ab+");

    if(fp)
    {
        fwrite(buffer, 1, block, fp);

        fflush(fp);
        fclose(fp);

        if(pTempName)
        {
            pTempName = NULL;
        }
        return block;
    }
    return 0;
#endif

}

//!
bool postUrl(char *filename,char *g_acHmServerAdDomain, char *field)
{
#if 1
    const int g_iHmServerAdCurlTimeout = 2500;
    int ret = -1;
    int info = 0;
    struct curl_slist *headers = NULL;
    CURL *hnd = NULL;
    char szUrl[256] = {0};
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    if (NULL == field || NULL == g_acHmServerAdDomain ){
        spdlog::info("init pointer is null exit...\n");
        return false;
    }
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL)
       return false;
    hnd = curl_easy_init();

    if (NULL == hnd){
        spdlog::info("init curl pointer failure...\n");
        return false;
    }

    memset(szUrl,0x00,sizeof(szUrl));
    //sprintf(szUrl, "%s?%s", g_acHmServerAdDomain, field);
    sprintf(szUrl, "%s", g_acHmServerAdDomain);
    spdlog::debug("send url: {}", szUrl);

    //headers = curl_slist_append(headers, "Content-Type: application/json");
    //headers = curl_slist_append(headers, "Accept: application/json");
    //curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);  ///>设置应用头 为json格式, php后台接收不到数据

    curl_easy_setopt(hnd, CURLOPT_URL, szUrl);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, hm_server_ad_fwrite); ///> 对返回的数据进行操作的函数地址
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk); ///> 这是 hm_server_ad_fwrite 的第四个参数值
    curl_easy_setopt(hnd, CURLOPT_POST, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, (void*)field);  /** Now specify the POST data */
    //curl_easy_setopt(hnd,CURLOPT_HEADER,1);                 ///> 将响应头信息和相应体一起传给write_data
    curl_easy_setopt(hnd, CURLOPT_VERBOSE,1);                ///> consol debug info

    //curl_easy_setopt(hnd, CURLOPT_TIMEOUT, g_iHmServerAdCurlTimeout);
    //curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, g_iHmServerAdCurlTimeout);
    //curl_easy_setopt(hnd, CURLOPT_NOSIGNAL, 1L);

    ret = curl_easy_perform(hnd);
    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE  , &info);
    if(ret != CURLE_OK || info != 200)
    {
        ret = 1;
        printf( "!crul failed to perform url='%s' res=[%d] rcode=%d\n",szUrl, ret,info);
        curl_easy_cleanup(hnd);
        free(chunk.memory);
        return false;
    }else{
        printf("%lu	bytes retrieved\n", (long)chunk.size);
        std::string str = chunk.memory;
        ret = fwrite(chunk.memory,1,chunk.size,fp);
        if( (ret < 0)||(ret != chunk.size) ){
            fclose(fp);
            spdlog::error("write {} file error..",filename);
            return false;
        }
        fclose(fp);
    }
    if(hnd)
       curl_easy_cleanup(hnd);

    free(chunk.memory);
    return true;
#endif

#if 0
    //! 此方法接收的post内容,无法全部写入文件。原因不祥待查。
    CURL *curl;
    CURLcode res;
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL)
        return false;
    curl = curl_easy_init();
    if (curl)
    {
        //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/cookie.txt");                  /// 指定cookie文件
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void*)field);       ///> 指定post body内容
        curl_easy_setopt(curl, CURLOPT_URL, g_acHmServerAdDomain);      ///> 指定url
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);            ///将返回的html主体数据输出到fp指向的文件
        curl_easy_setopt(curl,CURLOPT_VERBOSE,1);                ///> consol debug info
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        if (res != 0) {
            spdlog::info("post URL data download failure...\n");
            return false;
        }
        spdlog::info("post URL data  okay\n");
        return true;
    }
    fclose(fp);
    spdlog::info("init curl pointer failure...\n");
    return false;
#endif
}

//这个函数为CURLOPT_HEADERFUNCTION参数构造
/* 从http头部获取文件size*/
size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream) {

       int r,response;
       long len = 0;
       char str[1024],cmd5[128],lastTime[64];
       memset(cmd5,0,sizeof(cmd5));
       memset(lastTime,0,sizeof(lastTime));
       memset(str,'\0',sizeof(str));
       /* _snscanf() is Win32 specific */
       // r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
//       if(ptr != NULL)
//       printf("%s, line=%d, recive infor: %s \n",__FILE__,__LINE__,(char*)ptr);
//       if(stream != NULL)
//       printf("%s, line=%d, recive infor: %s \n",__FILE__,__LINE__,(char*)stream);
       strcpy(str,(char*)ptr);
        r = sscanf((const char*)ptr, "HTTP/1.1 %ld\n", &len);
        if (r){
           response = len;
           printf("%s, line=%d, HTTP response: %d \n\r",__FILE__,__LINE__,response);
        }
       r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
       if (r)
       {
          remain_file_length = len;
          printf("%s, line=%d, file length : %d \n\r",__FILE__,__LINE__,(int)remain_file_length);
       }
       r = sscanf((const char*)ptr, "Last-Modified: %32s", lastTime);
       if (r){
            printf("%s, line=%d, file last date: %s \n\r",__FILE__,__LINE__,lastTime);
       }
       r = sscanf((const char*)ptr, "Content-MD5: %s\n", cmd5);
       if (r)
          printf("%s, line=%d, file md5 check: %s \n\r",__FILE__,__LINE__,cmd5);

       return size * nmemb;
}

/* 保存下载文件 */
size_t wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
//    remain_file_length +=nmemb;
//    printf("file length:%d, seg lenth:%x \r\n",remain_file_length,(int32_t)nmemb);
        return fwrite((const void*)ptr, size, nmemb, (FILE*)stream);
}

/*读取上传文件 */
size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
       FILE *f = (FILE *)stream;
       size_t n;

       if (ferror(f))
              return CURL_READFUNC_ABORT;

       n = fread(ptr, size, nmemb, f) * size;

       return n;
}

/*  name        :  get file name
 *  breif       :
 * */
int getFileName(const char * remotepath,char *fname)
{
    char name[512],*ptr,*eptr;
    size_t st = strlen(remotepath);
    if(st > sizeof(name))
        return  -1;
    strcpy(name,remotepath);
    ptr = name;
    eptr = name;
    do{
    eptr += (ptr - eptr)+1;
    ptr = strchr(eptr,'/');
    }while(ptr != NULL);
    st -= (eptr - name);
    ptr = strncpy(fname,eptr,st);
    return 0;
}

/*
 *
 * 这个函数是为了符合CURLOPT_PROGRESSFUNCTION而构造的
 * 显示文件传输进度，t代表文件大小，d代表传 输已经完成部分
*/
#define TIMETYPE double

struct myprogress {
  TIMETYPE lastruntime; /* type depends on version, see above */
  CURL *curl;
  cmd_download_progress *msg;
};

//static curl_off_t remain_file_length = 0;
//static curl_off_t local_file_length = 0 ;
//static int progress_count = 0;

int manager_progress_func(
                     void   *ptr,
                     double t, /* dltotal */
                     double d, /* dlnow */
                     double ultotal,double ulnow)
{
 #if 1

  if(progress_count < 150)
      progress_count += 1;
  else{
      progress_count = 0;

      myprogress *progress = (myprogress*)ptr;
      cmd_download_progress *msg = progress->msg;
      msg->progress = (d + local_file_length)* 100 / (t+local_file_length);
      if(msg->progress < 100)
        msg->result = DOWNLOAD_ING;      //ing ...
      else
        msg->result = DOWNLOAD_SUCCESS;  //okay
      if(msg->device_type <= 3)      //! 3 is brain version max values
        cmd_download_progress_ui(msg);
      printf("msg->progress=%d \n",msg->progress);
      printf("\n dltotal = %d ; dlnow = %d  \n", (int)local_file_length+(int)t, (int)local_file_length+(int)d);
        //printf("\n ultotal = %d ; ulnow = %d  \n", (int)ultotal, (int)ulnow);
  }


#endif

#if 0
    printf("\n ultotal = %d / ulnow = %d  \n", ultotal, ulnow);
    printf("\n ==>>>> dltotal = %d / dlnow = %d ;== (%g %%)\n", d, t, d*100.0/t);
#endif

//  gdk_threads_enter();
//  gtk_progress_set_value(GTK_PROGRESS(ptr), d*100.0/t);
//  gdk_threads_leave();

  return 0;
}


// 下载 或者 上传文件函数
//!
//! \brief download
//! \param curlhandle
//! \param remotepath
//! \param localpath
//! \param timeout
//! \param type     :device type number
//! \return : CURLcode , CURLE_OK = 0 is success, > 0 is error note CURLcode define
//!
int download(CURL *curlhandle, const char * remotepath, const char * localpath,
           long timeout, uint8_t type)
{
    FILE *f;

    // long filesize =0 ;
    long filesize;
    CURLcode r = CURLE_GOT_NOTHING;
    CURLINFO info;

    cmd_download_progress msg;
    memset((char*)&msg,0,sizeof(msg));
    msg.device_type = type;
    struct myprogress prog;
    prog.curl = curlhandle;
    prog.msg  = &msg;

    struct stat file_info;
    int use_resume = 0;

    progress_count = 0;

    spdlog::debug(" {} {} ",__FILE__,__LINE__);

    //!采用追加方式打开文件，便于实现文件断点续传工作
    f = fopen(localpath, "ab+");
    if (f == NULL) {
        printf("%s, %d;open %s file faile\n",__FILE__,__LINE__,localpath);
        perror(NULL);
        return -2;
    }
    /** 得到本地文件大小,此函数不判断文件存在 */
    if(stat(localpath, &file_info) == 0)
    {
        local_file_length =  file_info.st_size;
        if(local_file_length !=0)
        {
            use_resume  = 1;
            spdlog::debug("resume file length: {} \r",local_file_length);
        }
    }

    //! set url infor & control parament
    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);
    //! 设置连接超时，单位秒
    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout);
    //! 下载文件总耗时 set time out s
    curl_easy_setopt(curlhandle, CURLOPT_TIMEOUT, download_timeout);
    //!将CURLOPT_VERBOSE属性设置为1，libcurl会输出通信过程中的一些细节。
    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

    //!如果仅仅是打印应答头的所有内容， 设置http 头部处理函数
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);

       /* config header control parament  */
    //!  curl_easy_setopt(curlhandle, CURLINFO_HEADER_OUT, true);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, true);
    // curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);

    //! 是否不需要响应的正文,为了节省带宽及时间,在只需要响应头的情况下可以不要正文
    //curl_setopt($oCurl, CURLOPT_NOBODY, true);

    //! 设置文件续传的位置给libcurl & 设置文件名
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_length:0);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, wirtefunc);

    /** set upload file name */
       //curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);
       //curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
       //curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);

    /** set load progress display , 进度响应函数 */
    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 0L);   // 0L progress
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSFUNCTION, manager_progress_func);
    curl_easy_setopt(curlhandle, CURLOPT_PROGRESSDATA, &prog);         //!数据传输的对象

    spdlog::debug(" {}, {} perform down load ..",__FILE__,__LINE__);
    //!   printf(" %s, %d;perform curl link \n",__FILE__,__LINE__);

  	r = curl_easy_perform(curlhandle);
  	
    filesize = curl_easy_getinfo(curlhandle, info,CURLINFO_HEADER_SIZE);

    spdlog::debug(" {}, {}, CURLcode = {},filelength:{}",__FILE__,__LINE__,r,filesize);
   //! printf("DEBUG %s, %d ,CURLcode = %d \n",__FILE__,__LINE__,r);
		
    fclose(f);
    if (r == CURLE_OK)
          return 0;
    else {
          fprintf(stderr, "%s\n", curl_easy_strerror(r));
          return r;
   }
}
/**
 * @brief down_load_file_entry
 * @param Path
 * @param url
 * @param type
 * @return      : CURLE_OK = 0 is success, > 0 is error note CURLcode define
 *                < 0 is user define errorCode
 */
int down_load_file_entry(char *Path, char* url,uint8_t type)
{
    char fname[128],fnamePath[128];

    char body[4096],header[1024];
    memset(fname,0,sizeof(fname));
    memset(fnamePath,0,sizeof(fnamePath));
    memset(header,0,sizeof(header));
    memset(body,0,sizeof(body));

    CURL *curlhandle = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curlhandle = curl_easy_init();

    if(curlhandle == NULL)
    {
        spdlog::error("{}, {}, curl handle failed \n",__FILE__,__LINE__);
        return -1;
    }
    spdlog::info(" {}, {}, curl init okay \n",__FILE__,__LINE__);
       
    int r = getFileName(url,fname);
    if(r != 0)
    {
        spdlog::error("{}, {}, url get file name error",__FILE__,__LINE__);
        return -2;
    }
    strcpy(fnamePath,Path);
    strcat(fnamePath,fname);
    r = download(curlhandle, url,fnamePath,time_out,type);
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();
    return r;
}

/**
 * @brief download_library_file, and unzip file
 * @param Path
 * @param url
 * @param type :device type number
 * @return     : CURLE_OK = 0 is success, > 0 is error note CURLcode define
 *              < 0 is user define errorCode
 */

int download_library_file(char *Path, char* url,uint8_t type)
{
    char fname[128],fnamePath[128];

    char body[4096],header[1024];
    memset(fname,0,sizeof(fname));
    memset(fnamePath,0,sizeof(fnamePath));
    memset(header,0,sizeof(header));
    memset(body,0,sizeof(body));

    CURL *curlhandle = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curlhandle = curl_easy_init();

    if(curlhandle == NULL)
    {
        spdlog::error("{}, {}, curl handle failed \n",__FILE__,__LINE__);
        return -1;
    }
    spdlog::info(" {}, {}, curl init okay \n",__FILE__,__LINE__);

    int r = getFileName(url,fname);
    if(r != 0)
    {
        spdlog::error("{}, {}, url get file name error",__FILE__,__LINE__);
        return -2;
    }
    strcpy(fnamePath,Path);
    strcat(fnamePath,fname);
    r = download(curlhandle, url,fnamePath,time_out,type);
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();
    if ( r == 0){//!unzip file
        char cmd[256];
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"unzip -o -d %s %s", Path, fnamePath);
        system(cmd);
        spdlog::debug(" run system cmd: {} ",cmd);
    }
    return r;
}

/**
    需要获取的是应答头中特定的信息，比如应答码、cookies列表等
    CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ... );
    info参数就是我们需要获取的内容，下面是一些参数值:
    1.CURLINFO_RESPONSE_CODE
    获取应答码
    2.CURLINFO_HEADER_SIZE
    头大小
    3.CURLINFO_COOKIELIST
    cookies列表
    除了获取应答信息外，这个函数还能获取curl的一些内部信息，如请求时间、连接时间等等。
*/

