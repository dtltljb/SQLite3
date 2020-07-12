#include <stdlib.h>

///downlaod
#include "down_load_file.hpp"
#include "infor_unpacket.hpp"
#include "version_infor.h"
#include "update_version.hpp"
#include "version_infor.h"
#include "forlder_util.h"
#include "ConfigVersionNumber.h"
#include "table_opration.h"
#include "select.h"

//
using json = nlohmann::json;
using namespace std;

BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_from_core;
BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_to_core;
BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_firmware_rx_from_serial_queue;
BlockingConcurrentQueue<std::vector<uint8_t>> rly_send_to_ui_queue;
version_infor infor;

extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;
extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_serial_queue;

//! \note  可以通过远程升级更换接口地址.
const char *d_get_url_base = "http://xx.xx.com/api/firmware/firmwareApi";
const char *d_post_url_base = "http://xx.xx.com/api/ku/kuapi";

const char *p_get_url_base = "http://xxxxx.cn/api/firmware/firmwareApi";
const char *p_post_url_base = "http://xxxx.cn/api/ku/kuapi";


char *get_url_base;
char *post_url_base;

//! \brief file path
const char *FILE_PATH_BASE = "/userdata/comment/";
const char *FILE_PATH_NAME = "/userdata/comment/comment.jscm";
const char *FILE_PATH_NAME_BAK = "/userdata/comment/comment_bak.jscm";
const char *FIRMWARE_CONFIG_PATH_FILE = "/userdata/comment/comment.jscm";
const char *LIBRARY_BASE_PATH = "/userdata/library";
const char *post_sava_path = "/userdata/library/post_in.jscm";
const char *monitor_progress = "/userdata/library/monitor.jscm";
const char *sys_config_file = "/userdata/library/brain.conf";
const char *device_catalog_file = "/userdata/library/version.json";

/** brain config file default content */
const char *_SN_DEFAULT = "ssss-20200218-00001";   //! ssss 缺省 SN 号
const char *_LIB_VER_DEF = "FFFFFFFFFFFFFFFFFF";
const char *LIBRARY_CREATE_TIME = "2020-02-25";

//!user define marco
#define	BUF_MAX_SIZE	2048
#define TO_UI_MSG_MAX_SIZE 4096


/**
 * @name     : update_firmware_entry()
 *
*/
//#define DEVICE_RUN_ENVIRONMENT 1
int update_firmware_entry(void)
{
     uint8_t buf[BUF_MAX_SIZE];
    //! \brief cloud url address
    ///> 1 测试调试服务器 , 0 产品发布服务器；在 cmakelist.txt 文件中修改此值
    if ( DEVICE_RUN_ENVIRONMENT == 1)
    {
        get_url_base = (char*)d_get_url_base;
        post_url_base = (char*)d_post_url_base;
    }
    else{
        get_url_base = (char*)p_get_url_base;
        post_url_base = (char*)p_post_url_base;
    }

    spdlog::debug("get_url_base:{}",get_url_base);
    spdlog::debug("post_url_base:{}",post_url_base);



    //! \part 2: check monitor progress file
    json js_monitor ={
        {"table_type",1},       //! type : 1 add_list ,2 edit_list,3 det_list
        {"mid",0},              //! mid : = 0 desc monitor file initialize status
        {"create_time",0},
        {"row",0},
        {"status", 1 },         //! initalize status = 1
        {"insert_row", " "},
        {"edit_row", " "},
        {"det_row", " "}
    };
    std::ifstream in_file(monitor_progress);
    std::string m_context = js_monitor.dump();
    if(!in_file.is_open()){
        infor.monitor_file_update(monitor_progress,m_context);
    }
    /**
     * @brief initialize interface content
    */
    json interface ={
        {"ku_version",_LIB_VER_DEF},
        {"brain_sn",_SN_DEFAULT},
        {"last_time","0"},
        {"page", 1 },
        {"length", 1 },
        {"ids",{" "} }   /** NOTE:object create mothed */
    };
    bool b;
    std::string field;
    int64_t stime;
    ret = infor.get_post_interface_time(post_sava_path, &stime);
    interface["last_time"] = stime;
    std::string js_interface = interface.dump();
    ret = infor.get_post_field_content( sys_config_file, js_interface, field);
    spdlog::info("postBody:{}",field);
    b = postUrl((char *)post_sava_path, (char*)post_url_base, (char*)field.data());
    if ( !b){
        spdlog::error("post url {} respose error..",post_url_base);
    }
    //! \note downlaod library content
    ret = recursively_download_library(post_sava_path, monitor_progress, sys_config_file, post_url_base );
    uint32_t n = (uint32_t)ret;
    if(n == 0){
        spdlog::info("okay");
    }
    //! \note OTA upgrade
    b = getUrl((char*)FILE_PATH_NAME,(char*)get_url_base);
    if(b != true){
        spdlog::error("down load {} file done failer...",FILE_PATH_NAME);
    }
}

/**
 * @brief recursively_download_library
 * @param d_name    :post_in.jscm
 * @param m_file    :monitor_progress.jscm
 * @param c_file    :brain configure.jscm
 * @param url       :url address
 * @return          :0 is download okay, > 0 list is empty, < 0  is error
 *
 * @part 1: 文件资源存放到本地 "/userdata/fileLibs/KEY_ID"
 *       2: 接口 flag = false 表示还有数据；再次检索表中写下。
 *       3: 0 is download okay, > 0 list is empty, < 0  is error
 */

int recursively_download_library(const char * d_name, const char * m_file,
                                          const char * c_file, char * url)
{
    /**
     * @note post interface content json
    */
    json interface ={
        {"ku_version",_LIB_VER_DEF},
        {"brain_sn",_SN_DEFAULT},
        {"last_time","0"},
        {"page", 1 },
        {"length", 1 },
        {"ids",{" "} }   /** NOTE:object create mothed */
    };

    cmd_download_progress msg;
    memset((char*)&msg,0,sizeof(msg));
    int type;
    int ret;
    int flag;
    bool b;
    std::string field,last_time;
    json js,js_m;

    std::ifstream in_file(d_name);
    std::string context,mContext;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",d_name);
        in_file.close();
        return -1;
    }
    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",d_name, size);
        return -2;
    }

    std::ifstream monitor_file(m_file);
    if(!monitor_file.is_open()){
        spdlog::debug("File open faile {}",m_file);
        monitor_file.close();
        return -3;
    }
    monitor_file.seekg(0,std::ios::end);
    size = monitor_file.tellg();
    monitor_file.seekg(0,std::ios::beg);
    mContext.assign(std::istreambuf_iterator<char>(monitor_file),std::istreambuf_iterator<char>());
    monitor_file.close();
    if(size < 1 ){
        spdlog::info(" {} file is empty,initialize status..",m_file);
        /**
         * @note initialize monitor file content
        */
        json js_monitor ={
            {"table_type",1},       //! type : 1 add_list ,2 edit_list,3 det_list
            {"mid",0},              //! mid : = 0 desc monitor file initialize status
            {"create_time",0},
            {"row",0},
            {"status", 1 },         //! initalize status = 1
            {"insert_row", " "},
            {"edit_row", " "},
            {"det_row", " "}
        };
        mContext = js_monitor.dump();
    }

    try{
        js = nlohmann::json::parse(context);
        flag = js["flag"];
        js_m = nlohmann::json::parse(mContext);
        type = js_m["table_type"];
        if(type > 3){
            spdlog::error("monitor file {} content ERROR!!",m_file);
            type = 1;
            js_m["table_type"] = type;
            std::string str = js_m.dump();
            infor.monitor_file_update(m_file,str);
        }
    }catch(std::exception & e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -4;
    }

    int count = js["data"]["add_list"].size();
    if ( count != 0 ){
        ret = infor.download_add_list_lib(d_name, m_file);
        if(ret < 0 ){
            spdlog::error(" download_add_list_lib() run error code: {}", ret);
        }
    }

    count = js["data"]["edit_list"].size();
    if(count != 0){
        ret = infor.download_edit_list_lib(d_name, m_file);
        if(ret < 0 ){
            spdlog::error(" download_edit_list_lib() run error code: {}", ret);
        }
    }

    count = js["data"]["delete_list"].size();
    if(count != 0){
        ret = infor.download_delete_list_lib(d_name, m_file);
        if(ret < 0 ){
            spdlog::error(" download_delete_list_lib() run error code: {}", ret);
        }
    }

    if(flag){
        /**
         * @note post request server get interface content
        */
        int page = js["page"];
        interface["last_time"] = js["last_time"];
        interface["page"] = page + 1 ;
        interface["length"] = 5;
        std::string js_interface = interface.dump();
        ret = infor.get_post_field_content( c_file, js_interface, field);
        spdlog::info("postBody:{}",field);
        b = postUrl((char *)d_name, url, (char*)field.data());
        if ( !b ){
            spdlog::error("post url {} respose error..", url);
            return -5;
        }
        recursively_download_library(d_name, m_file, c_file, url);

    }else{
        json js_config;
        std::string info;
        ret = infor.get_brain_config_info(c_file, info);
        try{
            js_config = json::parse(info);
            js_config["lib_version"] = js["ku_version"];
        }catch(std::exception & e){
            spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
            return -6;
        }
        info = js_config.dump();
        infor.set_brain_config_infor(c_file, info);
    }
    return 0;
}
































