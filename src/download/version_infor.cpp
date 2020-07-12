/*
*/
#include <sys/stat.h>
#include <dirent.h>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <map>
#include <vector>
#include <sys/stat.h>


//user header
#include "ipc_with_ui.hpp"
#include "lpc54113_firmware_updater.h"
#include "robot.hpp"
#include "unit.h"

#include "version_infor.h"
#include "forlder_util.h"
#include "down_load_file.hpp"
#include "ConfigVersionNumber.h"
#include "update_version.hpp"
//! table header
#include "table_opration.h"
#include "select.h"

extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;
extern version_infor infor;
extern Robot    robot;



#define X86_UBUNTU_DEBGU 1

#if X86_UBUNTU_DEBGU == 1
const char *SYSTEM_VERSION_PATH = "/proc/version";
#else
const char *SYSTEM_VERSION_PATH = "/proc/version_keyiinfo";
#endif


/*
 * @name         : find_comment_firmware_version
 * @brief        : user select agree ,检索 json 文件,
 * @note         : 检索 设备 运行版本信息。
 *
*/

int find_comment_firmware_version(uint8_t type,std::string & ver)
{

    std::map<uint8_t, std::string> version_map;

    const char *file_name = FILE_PATH_NAME;

    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
//    }catch(std::exception & e){
//        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
//        return -1;
//    }

    std::string str;
    int ver_type;

    int total_units_in_json = json_str["data"].size();
    int i = 0;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        ver_type= json_str["data"][i]["version_type"];
        if(ver_type != type)
        {
             continue;
        }
        //part 3: compare version number
        str = json_str["data"][i]["short_version"];                 /* get version number */
        ver = str;
        //memcpy(ver,(char *)str.data(),str.size());
        return 0;
    }
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
    return -2;
}


/*
 * @name     :
 * @brief    : brain compare device runing version number
 *            comment compare local *.bin.json file version number
 * @ret val  : 0 is equ, other is differ.
*/
static int mach_device_ver_num(int type,char *version)
{

    int ret = 0;                //default success debug use
    std::string str;
    str.clear();

    switch(type)
    {
    case    UPGRADE_BRAIN_IMAGE_TYPE: //brain update.img
    {
        str = infor.get_sys_version();
        ret = strcmp(version,(char*)str.data());
        //!debug
        //ret =_SAME_VERSION_FLAG;
        }
        break;
    case    UPGRADE_BRAIN_UI_TYPE://robot UI
    {
        str = infor.get_ui_version();
        ret = strcmp(version,(char*)str.data());
        //ret =_DIFF_VERSION_FLAG;
    }
        break;
    case    UPGRADE_BRAIN_CORE_TYPE: //robot brain app
    {
        str = infor.get_core_version();
        ret = strcmp(version,(char*)str.data());
        //ret =_DIFF_VERSION_FLAG;
    }
        break;
    case    UPGRADE_SUB_LCD_TYPE://sub and LCD
        /* no break */
    case    UPGRADE_JOINT_TYPE://joint

    case    UPGRADE_SKELETON_TYPE://

    case    UPGRADE_WHEEL_TYPE://wheel

    case    UPGRADE_CLAW_TYPE://claw

    case    UPGRADE_CUPULE_TYPE://cupule
    {
        ret = find_comment_firmware_version(type,str);
        if(ret == 0){
            spdlog::error("{},{}:non find comment firmware version",__FILE__,__LINE__);
            ret = 0;
            break;
        }
        ret = strcmp(version,(char*)str.data());
    }
        break;
    default:
        spdlog::error("{},{}:type {} mach failure",__FILE__,__LINE__,type);
        ret = 0;
        break;
    }
    spdlog::debug("type: {}, icloud: {}, local: {}, retVal:{}",type, version, str,ret);
    return ret;
}


/*
 * @name     : mach_device_run_ver_num
 * @brief    : compare joint\wheel runing version number
 *            comment compare local *.bin.json file version number
 * ret val  : 0 is equ, other is differ.
*/

static int mach_device_run_ver_num(uint8_t type,char *version,
                                   std::map<uint8_t, std::string> &version_map)
{

    int ret = 0;                //default success debug use
    std::string str;

    switch(type)
    {

    case    1://! joint  define in unit.hpp file

    case    2://!skeleton

    case    3://!wheel

    case    6://!claw

    case    7://!cupule

    case    8://sub lcd
    {
//        ret = find_comment_firmware_version(type,str);
//        if(ret != 0){
//            spdlog::error("{},{}:non find comment firmware version",__FILE__,__LINE__);
//            ret = -2;
//            break;
//        }
        std::map<uint8_t, std::string>::iterator iter;
        iter = version_map.find(type);
        if(iter == version_map.end())
        {
            spdlog::debug("don't mach {} device type..",type);
            ret = 0;       //! non mach device type
            break;
        }
        str = iter->second;
        //spdlog::debug("type:Version = {}:{}",iter->first,iter->second);
        ret = strcmp(version,(char*)str.data());
    }
        break;
    default:
        spdlog::error("{},{}:type {} mach failure",__FILE__,__LINE__,type);
        ret = 0;
        break;
    }
    spdlog::debug("retVal:{} type:{}, cloud:{},local:{}",ret, type, version, str);
    return ret;
}


/*
 * @name     :
 * @brief    : 检索 json["data"][i] 待升级文件,逐步升级固件
 *       (1): 如果有 brain 固件升级 先升级固件,在升级应用。
 *
 * ret val  : 0 is equ, other is differ.
 *
*/

static int mach_device_ver_update(json &js)
{
    char ve[8]={'n','u','l','l'};
    int ret = 0;                                //default success debug use
    memset(ve,0,sizeof(ve));
    char    fileName[64];
    char s_path[128];char d_path[128];
    char    url[256];
    std::string str,clound_version;
    memset(fileName,0,sizeof(fileName));
    memset(url,0,sizeof(url));
    memset(s_path,0,sizeof(s_path));
    memset(d_path,0,sizeof(d_path));
    char cmd[256];
    memset(cmd,0,sizeof(cmd));
    int type ;
    try{
    int status = js["status"];
    int seggest = js["version_add"];
    clound_version = js["short_version"];
    if( (status != UPDATE_FORCE)&&(seggest != AGREE_UPDATE) )
    {
        spdlog::debug("cloud version:{} , user give up update .. ",clound_version);
        return ret;
    }
    type = js["version_type"];
    str = js["version_file"];
    }catch(std::exception & ex){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, ex.what());
        return -1;
    }

    switch(type)
    {
    case    UPGRADE_BRAIN_IMAGE_TYPE:
        // VERSION_BASE_PATH
        std::sprintf(url,"%s",str.data());
        getFileName(url,fileName);                                  /*get file name */
        sprintf(s_path,"%s/%d/",VERSION_BASE_PATH,type);
        strcat(s_path,fileName);
        sprintf(cmd,"%s %s\n", SYS_UPDATE_CMD, s_path);
        spdlog::debug("\n {} system update start .... ",cmd);
        //!\note 修改为发送reboot命令给UI，UI需要显示到计时并执行update ota 命令
        cmd_core_to_ui_reboot msg;
        memset((char*)&msg, 0, sizeof(msg));
        msg.result = 1;                             //! : 1 update ota 复位通知,
        cmd_upgrade_reboot_notify_ui(&msg);
        //std::cout << cmd << "system update start ...." << '\n';
        //system(cmd);
        break;

    case    UPGRADE_BRAIN_UI_TYPE:
        // VERSION_BASE_PATH
        std::sprintf(url,"%s",str.data());
        getFileName(url,fileName);                                  /*get file name */
        sprintf(s_path,"%s/%d/",VERSION_BASE_PATH,type);
        strcat(s_path,fileName);
        sprintf(d_path,"%s/",PROCESS_BAS_PATH);
        strcat(d_path,fileName);
        sprintf(cmd,"cp -r %s %s \n",s_path,d_path);
        system(cmd);
        spdlog::debug("{} ,UI update start ....", cmd);
        //! chmod 755 file name
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"chmod 755 %s \n",d_path);
        system(cmd);
        spdlog::debug("UI update okay, {}",cmd);
        break;

    case    UPGRADE_BRAIN_CORE_TYPE:
        // VERSION_BASE_PATH
        std::sprintf(url,"%s",str.data());
        getFileName(url,fileName);                                  /*get file name */
        sprintf(s_path,"%s/%d/",VERSION_BASE_PATH,type);
        strcat(s_path,fileName);
        sprintf(d_path,"%s/",PROCESS_BAS_PATH);
        strcat(d_path,fileName);
        sprintf(cmd,"cp -r %s %s \n",s_path,d_path);
        system(cmd);
        spdlog::debug("{},Core update start ....",cmd);
        //! chmod 755 file name
        memset(cmd,0,sizeof(cmd));
        sprintf(cmd,"chmod 755 %s \n",d_path);
        system(cmd);
        spdlog::debug("Core update okay, {} ",cmd);
        break;

    case    UPGRADE_SUB_LCD_TYPE:
    {
        // VERSION_BASE_PATH
        std::sprintf(url,"%s",str.data());
        getFileName(url,fileName);                                  /*get file name */
        sprintf(s_path,"%s/%d/",VERSION_BASE_PATH,type);
        strcat(s_path,fileName);
        const std::string fileName = s_path;
        //upgrade_comment_firmware_file(fileName,type);
        robot.send_54113_lcd_motor_file(fileName,type);
        //robot.send_54113_lcd_motor_reset();
    }
        break;

    case    UPGRADE_JOINT_TYPE:
        break;
    case    UPGRADE_SKELETON_TYPE:
        break;
    case    UPGRADE_WHEEL_TYPE:
        break;
    case    UPGRADE_CLAW_TYPE:
        break;
    case    UPGRADE_CUPULE_TYPE:
        break;
    default:
        ret = -1;
        break;
    }
    spdlog::debug("cloud version:{},local version:{}",clound_version, ve);
    return ret;
}

/*
 * name     :
 * breif    : change device type number
 * ret val  : change type
*/
//TYPE_SUB_LCD = 8,

int version_infor::logic_to_upgrade_type(int logic_type)
{
    uint8_t ret;
    int type = logic_type;
    switch(type)
    {
    case    0://!brain
        ret = UPGRADE_BRAIN_IMAGE_TYPE;
        break;
    case    1://joint
        ret = UPGRADE_JOINT_TYPE;
        break;
    case    2://SKELETON
        ret = UPGRADE_SKELETON_TYPE;
        break;
    case    3://wheel
        ret = UPGRADE_WHEEL_TYPE;
        break;

    case    6://claw
        ret = UPGRADE_CLAW_TYPE;
        break;
    case    7://cupule
        ret = UPGRADE_CUPULE_TYPE;
        break;
    case    8://sub lcd
        ret = UPGRADE_SUB_LCD_TYPE;
        break;
    default:
        spdlog::error(" {},{}, unknow device type {} ...",
                       __FILE__,__LINE__,type);
        ret = -1;
        break;
    }
    return ret;
}

int version_infor::upgrade_to_logic_type(int upgrade_type)
{
    uint8_t ret;
    int type = upgrade_type;
    switch(type)
    {
    case    UPGRADE_BRAIN_IMAGE_TYPE:
        ret = 0;
        break;

    case    UPGRADE_BRAIN_UI_TYPE:
        ret = 0;
        break;
    case    UPGRADE_BRAIN_CORE_TYPE:
        ret = 0;
        break;


    case    UPGRADE_JOINT_TYPE:
        ret = 1;
        break;
    case    UPGRADE_SKELETON_TYPE:
        ret = 2;
        break;
    case    UPGRADE_WHEEL_TYPE:
        ret = 3;
        break;
    case UPGRADE_DISTOMAT_TYPE:
        ret = 4;
        break;
    case UPGRADE_CLAW_TYPE:
        ret = 6;
        break;
    case UPGRADE_CUPULE_TYPE:
        ret = 7;
        break;
    case    UPGRADE_SUB_LCD_TYPE:
        ret = 8;
        break;
    default:
        spdlog::error(" {},{}, unknow device type {} ...",
                      __FILE__,__LINE__,type);
        ret = -1;
        break;
    }
    return ret;
}

/*
 * @name         : find_comment_firmware_name
 * @breif        : version diff, return firmware file name
 * @input       : type is upgrade-type,upgrade version
 * @output      : char *output_firmware_file_name
 * @retval      : 0 is find meet a condition firmware file, other failure
 *
*/

int find_comment_firmware_name(uint8_t type, char *version, char *firmware_file_name)
{
    char *firmware_config_name = (char*)FILE_PATH_NAME;
    std::ifstream input_file(firmware_config_name);
    if(!input_file.is_open())
    {
        spdlog::error(" {},{}, open file faile",__FILE__,__LINE__);
        return -1;
    }
    if(firmware_config_name ==NULL)
    {
        spdlog::error(" {},{}, firmware_file_name is null",__FILE__,__LINE__);
        return -2;
    }
    try{
    json json_str ;
    input_file >> json_str;

    std::string str;
    uint8_t comment_type;
    int ret;
    char url[256];
    char fileName[64];
    char s_path[128];

    int total_units_in_json = json_str["data"].size();
    int i = 0;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: comment_type
        comment_type= json_str["data"][i]["version_type"];
        if(comment_type != type)
        {
            spdlog::debug(" input type:{},comment type:{} diffrent ,next device .. ", type, comment_type);
            continue;
        }
        else{
            //part 3: compare version,and get firmware file name
            str = json_str["data"][i]["short_version"];
            ret = strcmp(version,(char*)str.data());
            if(ret == _SAME_VERSION_FLAG)
            {
                spdlog::debug("device {} version:{} equ , don't upgrade ... ",type, str);
                return -3;
            }
            else{
                memset(url,0,sizeof(url));
                memset(fileName,0,sizeof(fileName));
                /* get file name from url link */
                str = json_str["data"][i]["version_file"];
                std::sprintf(url,"%s",str.data());
                getFileName(url,fileName);
                //marge file path and name
                memset(s_path,0,sizeof(s_path));
                sprintf(s_path,"%s/%d/",VERSION_BASE_PATH,type);
                strcat(s_path,fileName);
                //part 4: check file exist
                std::ifstream exist_file(s_path);
                if(!exist_file.is_open())
                {
                    spdlog::error("File open faile {}",s_path);
                    return -4;
                }
                strncpy(firmware_file_name, s_path, strlen(s_path)+1);
                return 0;
            }
        }
    }
    }catch(std::exception & e){
        spdlog::error("{} error not parse json {} ", __FILE__, e.what());
        return -6;
    }
    return -5;
}


/*
 * @name     : normalize_device_version_type
 * @brief    : compare upgrade dev type & version number
 * @input    : upgrade_type = logic_strucutre_map->get_type()
 * @output   : char *output_firmware_file_name( file name )
 * @ret val  :  0 is differ.other is equ
*/

 int version_infor::normalize_device_version_type(uint8_t logic_type, uint32_t ver_num, char *output_firmware_file_name)
{
     if(output_firmware_file_name ==NULL)
     {
         spdlog::error(" {},{}, open file faile",__FILE__,__LINE__);
         return -2;
     }

    char version[20];
    memset(version,0,sizeof(version));
    char fileName[64];
    memset(fileName,0,sizeof(fileName));
    int ret = 0;
    uint8_t V[4];
    //!little mode
    V[0] = (uint8_t)(ver_num>>24);
    V[1] = (uint8_t)(ver_num>>16);
    V[2] = (uint8_t)(ver_num>>8);
    V[3] = (uint8_t)(ver_num>>0);
    std::sprintf(version,"%d.%d.%d.%d",V[0],V[1],V[2],V[3]);
    int type = logic_to_upgrade_type(logic_type);
    spdlog::debug("ver num: {},upgrade type: {}",version,type);


    switch(type)
    {
    case    UPGRADE_SUB_LCD_TYPE://sub and LCD
//        ret = find_comment_firmware_name(type,version,fileName);
//        if(ret != 0 )
//         break;
//        strncpy(output_firmware_file_name,fileName,strlen(fileName)+1);
//        ret = 0;
//        break;
    case    UPGRADE_JOINT_TYPE://joint

    case    UPGRADE_SKELETON_TYPE://

    case    UPGRADE_WHEEL_TYPE://wheel

    case    UPGRADE_CLAW_TYPE://claw

    case    UPGRADE_CUPULE_TYPE://cupule
        ret = find_comment_firmware_name(type,version,fileName);
        if(ret != 0 ){
         spdlog::debug("type={}, version={},don't upgrade, next device.. ",type,version);
         break;
        }
        strncpy(output_firmware_file_name,fileName,strlen(fileName)+1);
        ret = 0;
        break;
    default:
        ret = -1;
        spdlog::error(" {},{}, unknow device type {} ...",__FILE__,__LINE__,type);
        break;
    }
    return ret;
}

/**
 * @name         : brain_update_firmware_agree
 * @brief        : 检索 json 文件, update brain system
 *			（1）: brain 征询意见作为1个整体来看。
 *
 *
*/
#define TRY_DOWNLOAD_MAX_COUNT  55
void version_infor::brain_upgrade_firmware(const char * file_name)
{
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
//    }catch(std::exception & e){
//        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
//        return;
//    }

    std::string str;
    int type;   int ret;
    cmd_update_progress msg;
    memset((char*)&msg,0,sizeof(msg));
    int total_units_in_json = json_str["data"].size();
    int i = 0;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        type= json_str["data"][i]["version_type"];
        if(type > UPGRADE_BRAIN_CORE_TYPE)
        {
            spdlog::debug(" {} type brain upgrade device,continue next device .. ",i);
            continue;
        }
        //part 3: compare version number
        str = json_str["data"][i]["short_version"];                 /* get version number */
        ret = mach_device_ver_num(type,(char*)str.data());
        if(ret == 0) //! 相同版本 或 错误都 放弃升级
        {
            spdlog::debug("type:{} version:{} give up upgrade firmware ,next device .. ",type, str);
            continue;
        }
        spdlog::debug("{} , ret val= {}, type = {}",__FILE__, ret, type);
        //part 4: send update brain progress
        msg.device_type = type;
        msg.progress    += 30;
        msg.result      = UPDATE_RESULT_ING;
        cmd_update_progress_ui(&msg);

        //part 5:done update brain process
        json_str["data"][i]["version_add"] = AGREE_UPDATE;
        json js = json_str["data"][i];
        mach_device_ver_update(js);

    }

    //part 8: send update okay
    msg.progress    = 100;
    msg.result      = UPDATE_RESULT_OKAY;
    cmd_update_progress_ui(&msg);
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return;
    }
    return;

}

//!
//! \brief version_infor::brain_download_firmware
//! \param file_name
//! \retval     : 0 download success, < 0 is failure
int version_infor::brain_download_firmware(const char * file_name)
{
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }

    try{
        context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
        json_str = nlohmann::json::parse(context);

        std::string str;
        std::string error_msg;
        int type;
        int ret;
        int download_failure_count = 0;
        cmd_download_progress d_msg;
        //memset((char*)&msg,0,sizeof(msg));
        int total_units_in_json = json_str["data"].size();
        int i = 0;
        for(i=0;i < total_units_in_json; i++)
        {
            //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
            type= json_str["data"][i]["version_type"];
            if(type > UPGRADE_SUB_LCD_TYPE)
            {
                spdlog::debug(" {} type brain download device,continue next device .. ",i);
                continue;
            }
            //part 3: compare version number
            str = json_str["data"][i]["short_version"];                 /* get version number */
            ret = mach_device_ver_num(type,(char*)str.data());
            if(ret == 0) //! 相同版本 或 错误都 放弃下载
            {
                spdlog::debug("type:{} version:{} give up download ,next device .. ", type, str);
                continue;
            }
            spdlog::debug("brain downlod device type={},ret val={}",type, ret);

            //part 4.1 : user agree update,download file
            char path[128];char tp[128];
            type=json_str["data"][i]["version_type"];
            memset(path,0,sizeof(path));
            //sprintf(path,"%s/%s/",VERSION_BASE_PATH,"tmp");
            sprintf(path,"%s/%d/",VERSION_BASE_PATH,type);
            ret = create_multi_dir(path);       /* create dir must be "%s/%d/" format */
            if(ret != 0)
            {
                spdlog::error("{},create_multi_dir is faile.. ",__FILE__);
                return -2;
            }

            //part 4.2: get download brain firmware url

            int cnt =0;
            str = json_str["data"][i]["version_file"];                  /* get url link */

            //part 4.3 rm -r tp/*
            memset(tp,0,sizeof(tp));
            sprintf(tp,"rm -r %s/%d/*",VERSION_BASE_PATH,type);
            spdlog::debug("{}",tp);
            system(tp); //delete forlder exist file

            do{
                ret = down_load_file_entry(path,(char*)str.data(),type);
                cnt +=1;
            }while(cnt < TRY_DOWNLOAD_MAX_COUNT && ret != 0);
            if( (cnt >= TRY_DOWNLOAD_MAX_COUNT)&&(ret != 0 ) )
            {
                download_failure_count += 1;
                error_msg.append(str);
                error_msg.append(" download fialure,failure count: ");
                error_msg.append(std::to_string(download_failure_count));
                spdlog::debug("device {} type down load {} failer,continue next device .. ", i, str);
                continue;
            }else{
                //part 5.2:send downlaod progress
                d_msg.progress = 100;
                d_msg.result = DOWNLOAD_SUCCESS;                            //success
                cmd_download_progress_ui(&d_msg);
            }
        }
        if (download_failure_count != 0 ){
            spdlog::error("{}",error_msg);
            return 1;
        }else{
            return 0;
        }

    }catch(std::exception & e){
        spdlog::error("version_infor.cpp, L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -3;
    }

}

/*
 * name     :
 * breif    : send upgrade comment request ui
*/

void version_infor::upgrade_comment_request_ui(const std::string & file_name,
                                               std::map<uint8_t, std::string> &version_map)
{
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
//    }catch(std::exception & e){
//        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
//        return;
//    }

    int total_units_in_json = json_str["data"].size();
    std::cout << total_units_in_json << '\n';
    if(total_units_in_json > MAX_VERSON_TOTAL)
    {
        spdlog::debug("{},MAX_VERSON_TOTAL is bigger.. ",__FILE__);
        return;
    }

    // part 1:
//    char path[128]; char url[512];char fileName[128];
//    int versionType;
    std::string str;
    int i = 0;
    int type;
    int ret;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        type= json_str["data"][i]["version_type"];
        if(type <= UPGRADE_BRAIN_CORE_TYPE )  //! include sub lcd UPGRADE_SUB_LCD_TYPE
        {
            spdlog::debug(" {} type comment don't upgrade device,continue next device .. ",i);
            continue;
        }
        //part 3: compare version number
        str = json_str["data"][i]["short_version"];                 /* get version number */
        type= json_str["data"][i]["version_type"];
        type = (uint8_t)upgrade_to_logic_type(type);           //! change unit.hpp file define type
        if(type == -1){
            spdlog::debug("device type {} don't mach ,next device .. ",type);
            continue;
        }
        ret = mach_device_run_ver_num(type,(char*)str.data(),version_map);
        if(ret == 0) //! 相同版本 或 错误都 放弃升级
        {
            spdlog::debug("type: {} version: {} give up update ,next device .. ",type, str);
            continue;
        }
        //part 4: ask for user suggest, = 0 is agree update, = 1 give up
        ret = json_str["data"][i]["version_add"];
        if(ret == 1)
        {
            spdlog::debug("device %d type user give up,continue next device .. ",ret);
            continue;
        }

        //part 5: send update firmware request to ui
        comment_upgrade_request msg;
        msg.status  =   json_str["data"][i]["status"];
        msg.device_type = json_str["data"][i]["build_version"];
        memset(msg.Ver,0,sizeof(msg.Ver));
        memcpy(msg.Ver,(char*)str.data(),str.size());

        //msg.desc_length = json_str["data"][i]["version_desc"].size();
        str =  json_str["data"][i]["version_desc"];
        msg.desc_length = str.size();
        memcpy(msg.desc,(char*)str.data(),sizeof(msg.desc));
        // send msg to ui
        cmd_upgrade_comment_request_ui(&msg);
        break;
    }
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return;
    }
    return;
}

/**
 * @brief version_infor::upgrade_comment_library_version
 * @param file      : post
 * @param version
 * @return
 *         in brain_config_file.conf file
 */
int version_infor::upgrade_comment_library_version(const char *file, std::string & version)
{

    std::ifstream input_file(file);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file);
        return -1;
    }

    try{
        context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
        json_str = nlohmann::json::parse(context);
        spdlog::info(" {} content: {} ",file, context);
        version = json_str["lib_version"];
        return 0;
    }catch(std::exception & e){
        spdlog::error("version_infor.cpp L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -2;
    }

}

/**
 * @brief version_infor::upgrade_brain_request_ui
 * @param file_name
 */
void version_infor::upgrade_brain_request_ui(const std::string & file_name)
{
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);

    int total_units_in_json = json_str["data"].size();
    std::cout << total_units_in_json << '\n';
    if(total_units_in_json > MAX_VERSON_TOTAL)
    {
        spdlog::debug("{},MAX_VERSON_TOTAL is bigger.. ",__FILE__);
        return;
    }

    // part 1:
//    char path[128]; char url[512];char fileName[128];
//    int versionType;
    std::string str;
    int i = 0; int type;
    int ret;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        type= json_str["data"][i]["version_type"];
        if(type > UPGRADE_BRAIN_CORE_TYPE)
        {
            spdlog::debug(" {} type upgrade brain request ui device,continue next device .. ",i);
            continue;
        }
        //part 3: compare version number
        str = json_str["data"][i]["short_version"];                 /* get version number */
        type= json_str["data"][i]["version_type"];
        ret = mach_device_ver_num(type,(char*)str.data());
        if(ret == 0) //! 相同版本 或 错误都 放弃升级
        {
            spdlog::debug("type:{} version:{} don't update ,next device .. ", type, str);
            continue;
        }
        spdlog::debug("upgrade request device type={},ret val={}",type, ret);
        //part 4: ask for user suggest, = 0 is agree update, = 1 give up
        ret = json_str["data"][i]["version_add"];
        if(ret == 1)
        {
            spdlog::debug("device {} type user give up,continue next device .. ",ret);
            continue;
        }

        //part 5: send update firmware request to ui
        cmd_update_request msg;
        msg.status  =   json_str["data"][i]["status"];
        msg.device_type = json_str["data"][i]["build_version"];
        memset(msg.Ver,0,sizeof(msg.Ver));
        memcpy(msg.Ver,(char*)str.data(),str.size());

        //msg.desc_length = json_str["data"][i]["version_desc"].size();
        str =  json_str["data"][i]["version_desc"];
        msg.desc_length = str.size();
        memcpy(msg.desc,(char*)str.data(),sizeof(msg.desc));
        // send msg to ui
        cmd_upgrade_request_ui(&msg);
        break;
    }
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return ;
    }
    return;
}

//!
//! \brief version_infor::download_brain_request_ui
//! \param file_name
//! \return : 0 version unchange, 1 version change, < 0 is error
//!
int version_infor::download_brain_request_ui(const std::string & file_name)
{
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
//    }catch(std::exception & e){
//        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
//        return;
//    }

    int total_units_in_json = json_str["data"].size();
    std::cout << total_units_in_json << '\n';
    if(total_units_in_json > MAX_VERSON_TOTAL)
    {
        spdlog::debug("{},MAX_VERSON_TOTAL is bigger.. ",__FILE__);
        return -2;
    }

    //! \part 1:
//    char path[128]; char url[512];char fileName[128];
//    int versionType;
    std::string str;
    int i = 0; int type;
    int ret;
    for(i=0;i < total_units_in_json; i++)
    {
        //!\part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        type= json_str["data"][i]["version_type"];
        if(type > UPGRADE_BRAIN_CORE_TYPE)
        {
            spdlog::debug(" {} type download_brain_request_ui ,continue next device .. ",i);
            continue;
        }
        //!\part 3: compare version number
        str = json_str["data"][i]["short_version"];                 /* get version number */
        type= json_str["data"][i]["version_type"];
        ret = mach_device_ver_num(type,(char*)str.data());
        if(ret == 0) //! 相同版本 或 错误都 放弃下载
        {
            spdlog::debug("type:{} version:{} give up download ,next device .. ",str);
            continue;
        }
        spdlog::debug(" downlod brain request, type={},ret val={}",type, ret);
        //!\part 4: ask for user suggest, = 0 is agree update, = 1 give up
        ret = json_str["data"][i]["version_add"];
        if(ret == 1)
        {
            spdlog::debug("device {} type user give up download,continue next device .. ",ret);
            continue;
        }


        //!\part 5: send download firmware request to ui
        cmd_update_request msg;
        msg.status  =   json_str["data"][i]["status"];
        msg.device_type = json_str["data"][i]["build_version"];
        memset(msg.Ver,0,sizeof(msg.Ver));
        memcpy(msg.Ver,(char*)str.data(),str.size());

        //msg.desc_length = json_str["data"][i]["version_desc"].size();
        str =  json_str["data"][i]["version_desc"];
        msg.desc_length = str.size();
        memcpy(msg.desc,(char*)str.data(),sizeof(msg.desc));
        /// send msg to ui
        cmd_download_request_ui(&msg);
        break;
    }
    //! \part 6: version unchange notify to ui
    if (i >= total_units_in_json ){
        return 0;
        //cmd_version_unchange_notify_ui();
    }
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -3;
    }
    return 1;//! version change
}

//!
//! \brief version_infor::download_library_request_ui
//! \param file_name
//! \return :
//!
int version_infor::download_library_request_ui(const std::string & file_name)
{
    std::ifstream input_file(file_name);
    std::string context;
    json js ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }

    try{
        context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
        js = nlohmann::json::parse(context);

        //! \part 1: add_list content count
        int add_list_in_json = js[j_data][j_add_list].size();
        if ( add_list_in_json > 0){
            std::string str;
            cmd_update_request msg;
            msg.status  =   0;

            str =  js[j_data][j_add_list][0][file_desc];
            msg.desc_length = str.size();
            memcpy(msg.desc,(char*)str.data(),sizeof(msg.desc));

            str = js[j_data][j_add_list][0][f_version];
            memset(msg.Ver,0,sizeof(msg.Ver));
            memcpy(msg.Ver,(char*)str.data(),str.size());
            msg.device_type = (uint8_t)js[j_data][j_add_list][0][f_lib_para];

            cmd_download_request_ui(&msg);
            return 0;
        }

        int edit_list_in_json = js[j_data][j_edt_list].size();
        if ( add_list_in_json > 0){
            std::string str;
            cmd_update_request msg;
            msg.status  =   0;

            str =  js[j_data][j_add_list][0][file_desc];
            msg.desc_length = str.size();
            memcpy(msg.desc,(char*)str.data(),sizeof(msg.desc));

            str = js[j_data][j_add_list][0][f_version];
            memset(msg.Ver,0,sizeof(msg.Ver));
            memcpy(msg.Ver,(char*)str.data(),str.size());
            msg.device_type = (uint8_t)js[j_data][j_add_list][0][f_lib_para];

            spdlog::debug("add_list context count={}",edit_list_in_json);
            cmd_download_request_ui(&msg);
            return 0;
        }

        int delete_list_in_json = js[j_data][j_del_list].size();
        if ( delete_list_in_json > 0){

            spdlog::debug("add_list context count={}",delete_list_in_json);
            cmd_version_unchange_notify_ui();
            return 0;
        }

    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -2;
    }
}


//!
//! \brief version_infor::get_json_to_file
//! \param file_name
//! \return
//! \note :download comment upgrade file
int version_infor::get_json_to_file(const std::string & file_name)
{
    std::this_thread::sleep_for(std::chrono::seconds(1)); //wait for sys file write
    std::ofstream ofs;
    std::ifstream input_file(file_name);
    std::string context;
    json json_str,json_bak;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }
    input_file.seekg(0, std::ios::end);
    int nFileLen = input_file.tellg();
    if( nFileLen <= 2){
        spdlog::error(" {} file size too few,...",file_name);
        return -2;
    }

    try{
         input_file.seekg(0, std::ios::beg);
         context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
         json_str = nlohmann::json::parse(context);

        // read-only access
        std::cout << json_str.at("/result"_json_pointer) << '\n';
        std::cout << json_str.at("/msg"_json_pointer) << '\n';
        // output element with JSON pointer "/number"
        std::cout << json_str.at("/data"_json_pointer) << '\n';

        int total_units_in_json = json_str["data"].size();
        std::cout << total_units_in_json << '\n';
        if(total_units_in_json > MAX_VERSON_TOTAL)
        {
            spdlog::debug("{},MAX_VERSON_TOTAL is bigger.. ",__FILE__);
            return -3;
        }

        //! \part 1:
        char path[128];
        //int versionType;
        std::string str;
        int i = 0;
        int type;
        int ret;
        for(i=0;i < total_units_in_json; i++)
        {
            //! \part 2: creat device type forlder
            type = json_str["data"][i]["version_type"];
            memset(path,0,sizeof(path));
            sprintf(path,"%s/%d/",VERSION_BASE_PATH,type);
            ret = create_multi_dir(path);       /* create dir must be "%s/%d/" format */
            if(ret != 0)
            {
                spdlog::error("{},create_multi_dir is faile.. ",__FILE__);
                return -4;
            }

            //! \part 3: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
            if(type <= UPGRADE_BRAIN_CORE_TYPE)
            {
                spdlog::debug(" %d type less than COMMENT device,continue next device .. ",i);
                continue;
            }

            //! \part 4: down load comment firmware process file
            int cnt = 0;
            char url[512];
            char fileName[128];
            memset(url,0,sizeof(url));
            str = json_str["data"][i]["version_file"];              /** get url link */
            std::sprintf(url,"%s",str.data());

            //! \note mach version number equ
            char tmp[128];
            memset(tmp,0,sizeof(tmp));
            sprintf(tmp,"%s/%d/",VERSION_BASE_PATH, type);
            memset(fileName,'\0',sizeof(fileName));
            getFileName(url,fileName);                              /** get file name */
            strcat(tmp,fileName);
            strcat(tmp,".json");                                    /** get comment firmware information */
            std::ifstream in(tmp);
            json js;
            std::string cloud_ver,local_ver;
            cloud_ver = json_str["data"][i]["short_version"];
            if ( in.is_open() ){
                context.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
                js = nlohmann::json::parse(context);
                local_ver = js["short_version"];
            }
            if ( cloud_ver == local_ver ){
                spdlog::info(" cloud version: {}, local version: {} ,next ...",cloud_ver, local_ver);
                continue;
            }

            //! \part 4.1 rm -r tp/*
            char cmd[128];
            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"rm -r %s/%d/*",VERSION_BASE_PATH,type);
            spdlog::debug("system call: {} ",cmd);
            system(cmd);                                    //!delete forlder exist file
            std::this_thread::sleep_for(std::chrono::seconds(1));
            do{
                ret = down_load_file_entry(path,(char*)str.data(),type);
                cnt +=1;
            }while(cnt < 3 && ret != 0);
            if( (cnt >= 3)&&(ret != 0 ) )
            {
                spdlog::debug("device %d type down load file failer,continue next device .. ",i);
                spdlog::debug("name:{}",str.data());
                continue;
            }

            json_str["data"][i]["version_level"]= 1;          /* set down load store flag */
            //! \part 5: store device version information json_bak.txt

            strcat(path,fileName);
            strcat(path,".json");
            ofs.open(path);
            if(!ofs.is_open())
            {
                perror("file open failure..");
                spdlog::error("create {} file failure ... ",fileName);
                return -5;
            }
            ofs << json_str["data"][i];
            //json_str["data"][i] >> ofs;
            ofs.close();
        }
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -6;
    }
    return 0;
}

int version_infor::get_post_interface_time(const char *p_file, int64_t *time)
{
    std::ofstream ofs;
    std::ifstream input_file(p_file);

    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",p_file);
        return -1;
    }
    input_file.seekg(0, std::ios::end);
    int nFileLen = input_file.tellg();
    input_file.seekg(0, std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();
    if( nFileLen <= 2){
        spdlog::error(" {} file size too few,...",p_file);
        return -2;
    }

    try{

        json_str = nlohmann::json::parse(context);
        *time = json_str["last_time"];
        //spdlog::info(" server response last time: {} ", time);
        std::cout << "server response last time: " << time << '\n';
        return 0;

    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -3;
    }

}

/**
 * @brief version_infor::post_json_to_file
 * @param file_name
 * @return  : 0 is have data context; 1 is empty, other is error
 * @note    : query add_list[] || update_list[] don't empty ; it have information
 */
int version_infor::post_json_to_file(const std::string & file_name)
{
    std::this_thread::sleep_for(std::chrono::seconds(1)); //wait for sys file write
    std::ofstream ofs;
    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }
    input_file.seekg(0, std::ios::end);
    int nFileLen = input_file.tellg();
    if( nFileLen <= 2){
        spdlog::error(" {} file size too few,...",file_name);
        return -2;
    }
    input_file.seekg(0, std::ios::beg);

    try{

     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
    input_file.close();
    // read-only access
    std::cout << json_str.at("/result"_json_pointer) << '\n';
    std::cout << json_str.at("/msg"_json_pointer) << '\n';
    std::cout << "add_list:" << json_str.at("/data/add_list"_json_pointer) << '\n';
    std::cout << "edit_list:" << json_str.at("/data/edit_list"_json_pointer) << '\n';
    std::cout << "delete_list:" << json_str.at("/data/delete_list"_json_pointer) << '\n';
    std::cout << "flag:" << json_str.at("/flag"_json_pointer) << '\n';

    int total_list_in_json = json_str["data"].size();
    std::cout << total_list_in_json << '\n';
    if(total_list_in_json > 4)
    {
        spdlog::debug("{},MAX_LIST_COUNT is bigger.. ",__FILE__);
        return -3;
    }

    //! \part 1: add_list content count
    int add_list_in_json = json_str["data"]["add_list"].size();
    if ( add_list_in_json > 0){
        spdlog::debug("add_list context count={}",add_list_in_json);
        return 0;
    }

    int edit_list_in_json = json_str["data"]["edit_list"].size();
    if ( add_list_in_json > 0){
        spdlog::debug("add_list context count={}",edit_list_in_json);
        return 0;
    }

    int delete_list_in_json = json_str["data"]["delete_list"].size();
    if ( add_list_in_json > 0){
        spdlog::debug("add_list context count={}",delete_list_in_json);
        return 0;
    }
    return 1;
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -4;
    }

}
/*
 * name     :
 * breif    : save_ui_version info
 * output   : char *
 * ret      : none
*/
void version_infor::save_ui_version( char *pVer,uint8_t len )
{
    if( (len ==0)||(pVer == NULL) ){
        spdlog::debug("{},line={},pVer = NULL \n",__FILE__,__LINE__);
        return;
    }
    pVer[len-1] = '\0';
    ui_ver.clear();
    int i=0;
    for(;i<len;i++)
        ui_ver.push_back(pVer[i]);
    spdlog::debug("ui version:{} length:{}",ui_ver,ui_ver.size());
    return;
}

/*
 * name     :
 * breif    : get_ui_version
 * ret      : std::string
*/
std::string version_infor::get_ui_version( void )
{
    return this->ui_ver;
}

/*
 * name     :
 * breif    : get_sys_version
 * ret      : std::string
*/
std::string version_infor::get_sys_version( void )
{
    return this->sys_ver;
}

/*
 * name     :
 * breif    : get_core_version
 * ret      : std::string
*/
std::string version_infor::get_core_version( void )
{
    return this->core_ver;
}

/*
 * name     :
 * breif    : get_brain_version
 * output   : char *
 * ret      : none
*/

void version_infor::get_brain_version(void)
{
    // 读取 core 版本信息
    // 读取 linux 版本信息
    // 读取 ui 版本信息
    char    *sys_version = (char*)SYSTEM_VERSION_PATH;
    int     fd,length_r;
    char	buffer[512];
    char 	*sPtr,*ePtr;
    std::string str;
    //get brain core version
    memset (buffer,'\0',sizeof(buffer));
    sprintf(buffer,"%d.%d.%d.%d",
                    BRAIN_VERSION_VENDOR,BRAIN_VERSION_MAJOR,BRAIN_VERSION_MINOR,BRAIN_VERSION_PATCH);
    core_ver.clear();
    uint8_t i=0;
    for(;i<strlen(buffer);i++)
        core_ver.push_back(buffer[i]);
    core_ver.push_back(0);

    //get brain sys version
    fd = open(sys_version,O_RDONLY,0777);
    if( fd < 0 ) {
           printf("open %s failed!\n",sys_version);
           //exit (1);
           return ;
       }
    length_r = read(fd,buffer,sizeof(buffer) );
    close(fd);

    sPtr	=   buffer; //input prm
    sPtr 	= strstr(sPtr,"version:");
    if(sPtr  ==  NULL){
        memcpy((char *)str.data(),buffer,length_r);
        spdlog::debug("version:{} don't mach ",str);

    }else{
            sPtr	+=	sizeof("version:")-1;
            ePtr	=	strchr(sPtr,' ');
            if(ePtr == NULL){
            spdlog::debug("{},line={},non enter key, exit\n",__FILE__,__LINE__);
            }
         //strncpy((char*)sys_ver.data(),sPtr,ePtr-sPtr);
            sys_ver.clear();
            int len;
            if(ePtr-sPtr <= 18)
                len = ePtr-sPtr;
            else
                len = 18;
            int i=0;
            for(;i<len;i++)
                sys_ver.push_back(sPtr[i]);
         spdlog::debug(">>>>> system Version:{},",sys_ver);
    }

    //send get brain ui version commad
//    cmd_ui_version msg;
//    msg.result  = 1;
//    cmd_get_version_to_ui(&msg);
}

void version_infor::read_brain_ui_version(void)
{
    //send get brain ui version commad
    cmd_ui_version msg;
    msg.result  = 1;
    cmd_get_version_to_ui(&msg);
}

/*
 * name     : get_local_total_version
 * output   : cmd_core_to_ui_total_version * msg
 *            uint8_t *count
 * retval   : 0 success, other is failure
 *
*/
int version_infor::get_local_total_version(cmd_core_to_ui_total_version * msg,uint8_t *count)
{
    std::string file_name = FILE_PATH_NAME;

    if(msg == NULL )
    {
        spdlog::error(" {},{},* msg = NULL ... ",__FILE__,__LINE__);
        return -2;
    }

    std::ifstream input_file(file_name);
    std::string context;
    json json_str ;
    if(!input_file.is_open())
    {
        spdlog::debug("File open faile {}",file_name);
        return -1;
    }

    try{
     context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
     json_str = nlohmann::json::parse(context);
//    }catch(std::exception & e){
//        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
//        return -4;
//    }

    int total_units_in_json = json_str["data"].size();
    std::cout << total_units_in_json << '\n';
    if(total_units_in_json > MAX_VERSON_TOTAL)
    {
        spdlog::debug("{},MAX_VERSON_TOTAL is bigger.. ",__FILE__);
        return -3;
    }

    //! part 1:local firmware information
    std::string str;
    int i = 0;
    for(i=0;i < total_units_in_json; i++)
    {
        //part 2: version <= 3,include: 3=brain_core_version,2=brain_ui,1=brain_system
        msg->sebVersion[i].device_type = json_str["data"][i]["version_type"];
        str = json_str["data"][i]["short_version"];                 /* get version number */
        memcpy(msg->sebVersion[i].Ver,(char*)str.data(),str.size());

        str =  json_str["data"][i]["version_desc"];
        msg->sebVersion[i].desc_length = str.size();
        memcpy((char*)msg->sebVersion[i].desc,(char*)str.data(),sizeof(msg->sebVersion[i].desc));
    }
    *count = total_units_in_json;
    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
    return 0;
}

/*
 * name     ：
 * breif    ：检索 comment下个文件夹内的 json 文件,获取程序文件下载需求,
 *           直接下载对应的文件,同步固件版本。
 *          json 文件中输出  固件版本升级标识。
 *
 * current file:
 *              *.jscm
 * bakup    file:
 *              *.json
*/

void version_infor::find_dir_key_word(const char *key,const char * base_path, std::vector<std::string> &list)
{

    struct dirent *dp;
    DIR *dir = opendir(base_path);
    char path[128];
    memset(path,0,sizeof(path));

    if (!dir) {
        spdlog::error("Open DIR failed: {}", base_path);
        std::cout << "don't find file " << '\n';
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            //spdlog::debug("{}", dp->d_name);
            //std::cout << dp->d_name << '\n';
            strcpy(path, base_path);
            strcat(path, "/");
            strcat(path, dp->d_name);

            if (DT_DIR == dp->d_type) {
                //spdlog::debug("DIR: {}", path);
                find_dir_key_word(key,path, list);
            }
            else if (DT_REG == dp->d_type) {
                if (strstr(dp->d_name,key) != nullptr) {
                    std::string file_name_string(path);
                    list.push_back(file_name_string);
                    spdlog::debug("File: {}", path);
                }
            }
        }
    }
    closedir(dir);
}


/**
 * @brief version_infor::get_brain_config_info
 * @param c_file    :config file
 * @param info      :config file content
 * @return          :0 success
 */
int version_infor::get_brain_config_info(const char * c_file, std::string &info)
{
    json j_conf =
    {
        {"brain_sn","keyi-20200215-0001"},
        {"lib_version","1.0.0"},
        {"create_time","2020-02-25"},
        {"product_flag",0},         //! flag = 0 none initialize parament
        {"power_on",1}              //! power_on = 1 first power on brain
    };

    std::string content;
    //! \note check file exist
    std::ifstream config(c_file);
    std::ofstream ofs;
    if(!config.is_open() ){
        ofs.open(c_file);
        ofs << j_conf.dump();
        ofs.close();
        spdlog::info("初始化 brain 首次开机配置文件 {}",c_file);
        info = j_conf.dump();
        return 0;               //!first open maching
    }
    config.seekg(0,std::ios::end);
    int size = config.tellg();
    config.seekg(0,std::ios::beg);
    content.assign(std::istream_iterator<char>(config),std::istream_iterator<char>());
    config.close();

    if(size < 1){
        ofs.open(c_file);
        ofs << j_conf.dump();
        ofs.close();
        spdlog::info(" brain 开机配置文件损坏，恢复为出厂缺省参数 {}",c_file);
        info = j_conf.dump();
        return 0;  //!first open maching
    }

    try{
        json js = nlohmann::json::parse(content);
        info = js.dump();
        return 0;
    }catch(std::exception & e){
        spdlog::error("version_infor.cpp,l:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
}

int version_infor::set_brain_config_infor(const char * c_file, std::string &j_str)
{
    try{
        std::ofstream o_file;
        std::string out = j_str;
        o_file.open(c_file);
        o_file << out;
        o_file.close();
    }catch(std::exception & e){
        spdlog::error("version_infor.cpp,l:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
    return 0;
}

/**
 * @brief version_infor::monitor_file_update
 * @param f_name
 * @param js
 * @return
 */

int version_infor::monitor_file_update(const char *f_name, std::string &j_str)
{

    try{
        std::ofstream o_file;
        std::string out = j_str;
        o_file.open(f_name);
        o_file << out;
        o_file.close();
    }catch(std::exception & e){
        spdlog::error("version_infor.cpp,l:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
    return 0;
}

/**
 * @brief version_infor::get_post_field_content
 * @param f_name    :brain config file
 * @param interface :interface json
 * @param field     :post field content
 * @return      : 0 success
 */
int version_infor::get_post_field_content(const char *c_file, std::string &j_interface, std::string &field)
{
    int ret;
    //! \brief init post input param
    std::string str;
    field = "param=";

    try{

        json js = json::parse(j_interface);
        std::string res ;
        ret = infor.get_brain_config_info(c_file, res);

        if(res.size() == 0){
            js["ku_version"] = BRAIN_LIB_VER_DEF;
            js["brain_sn"] = BRAIN_SN_DEFAULT;
        }else{
            json js_res = json::parse(res);
            js["ku_version"] = js_res["lib_version"];
            js["brain_sn"] = js_res["brain_sn"];
        }

        const char *key = "mid";
        std::vector<std::string> mid_list;
        query_total_table_mid((char*)key, mid_list);
        int len = mid_list.size();
        if ( len == 0){
            js["ids"][0] = "";
        }else{
            for(int i=0; i < len; i++){
                js["ids"][i] = mid_list.at(i);
            }
        }
        str = js.dump();
        field.append(str);
    }catch(std::exception & e){
        spdlog::error("version_infor.cpp {}, ERR:can not parse json: {}",__LINE__,e.what());
        field.clear();
        return -1;
    }
    return 0;
}

/**
 * @brief version_infor::download_add_list_lib
 * @param d_name :download library file
 * @param m_file :monitor progress file
 * @return       :0 is download okay, > 0 list is empty, < 0  is error
 */
int version_infor::download_add_list_lib(const char * d_name, const char * m_file )
{

    std::string context,mContext;
    json js, js_m ;
    std::ifstream input_file(d_name);
    if(!input_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, d_name);
        return -1;
    }
    input_file.seekg(0,std::ios::end);
    int size = input_file.tellg();
    input_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();
    if (size < 10 ){
        spdlog::error(" line: {} file {} content too few, abort exit..", __LINE__, d_name);
        return -2;
    }

    std::ifstream monitor_file(m_file);
    if(!monitor_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, m_file);
        monitor_file.close();
        return -3;
    }
    monitor_file.seekg(0,std::ios::end);
    size = monitor_file.tellg();
    monitor_file.seekg(0,std::ios::beg);
    mContext.assign(std::istreambuf_iterator<char>(monitor_file),std::istreambuf_iterator<char>());
    monitor_file.close();
    if(size < 1 ){
        spdlog::error(" line:{}, file {} content is empty, abort exit..",__LINE__, m_file);
        return -4;
    }
    std::map<uint8_t, std::string> insert_error_map;
    std::map<uint8_t, std::string>::iterator it;
    std::string err_msg;
    std::string str,tableName;
    int type;
    int ret;
    int insert_row_count = 0 ;
    cmd_update_progress msg;
    cmd_download_progress d_msg;
    memset((char*)&msg,0,sizeof(msg));

    try{
        js = nlohmann::json::parse(context);
        js_m = nlohmann::json::parse(mContext);
//        type = js_m["table_type"];
//        if( type != 1){
//            spdlog::error(" line:{}, table type {} don't mach add_list, abort exit..",__LINE__, type);
//            return -5;
//        }

        int add_list_count = js[j_data][j_add_list].size();
        if( add_list_count == 0 ){
            spdlog::info("file {} content add_list[] is empty,exit..",m_file);
            return 1;
        }

        int i = js_m["row"];                //! \note :monitor row pass to i = row
        if (i > add_list_count){
            spdlog::info("line: {}, file {} content row is ERROR,abort exit..", __LINE__, m_file);
            return 2;
        }

        for( ; i < add_list_count; i++)
        {
            //! \part 2: role scenery get table name
            int mid = js[j_data][j_add_list][i][key_id];
            std::string name = js[j_data][j_add_list][i][file_name];
            type= js[j_data][j_add_list][i][role_scenery];
            tableName = "NORMAL_ACTION";
//            switch (type) {
//            case 1:
//                tableName = "NORMAL_ACTION";
//                break;
//            case 2:
//                tableName = "SLEEP_ACTION";
//                break;
//            case 3:
//                tableName = "EVENT_ACTION";
//                break;
//            default:
//                tableName.clear();
//                break;
//            }

            //! \part 3: insert row to table
            if (tableName.size() == 0){
                err_msg = "下发数据的表类型错误,联系运营人员,文件名称:";
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -3;
            }
            ret = table_insert_row( (char*)tableName.data(), js, (char*)j_add_list, i );
            if ( ret < 0 ){
                err_msg = "数据库插入错误,错误代码： ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -4;
            }

            //!\part 4.1: create download resource path
            char path[128];
            ret = js[j_data][j_add_list][i][key_id];
            memset(path,0,sizeof(path));
            sprintf(path,"%s/%d/",LIBRARY_BASE_PATH,ret);
            ret = create_multi_dir(path);       /* create dir must be "%s/%d/" format */
            if(ret != 0)
            {
                err_msg = "create forld error,code: ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -5;
            }

            type = js[j_data][j_add_list][i][f_lib_para];       //!device type
            str = js[j_data][j_add_list][i][url_path];
            int cnt = 0 ;
            //! \part 4.2: get download brain firmware url
            do{
                ret = download_library_file(path,(char*)str.data(),type);
                cnt +=1;
            }while( cnt < 5 && ret != 0);

            if( ( cnt >= 5 ) && ( ret != 0 ) )
            {
                //!\part 5.1:send  downlaod failure
                d_msg.device_type = type;                       ///> device type
                d_msg.progress = 0;
                d_msg.result = DOWNLOAD_FAILUR;
                cmd_download_progress_ui(&d_msg);

                err_msg = "download file failure,code: ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
            }else{
                insert_row_count += 1;
                /** @note monitor file content saved */
                js_m["row"] = i ;
                js_m["mid"] = js[j_data][j_add_list][i][key_id];
                std::string s = js_m.dump();
                monitor_file_update(m_file,s);
                //!\part 5.2:send downlaod progress
                d_msg.device_type = type;                  ///> device type
                d_msg.progress = 100;                
                d_msg.result = DOWNLOAD_SUCCESS;        ///> success
                cmd_download_progress_ui(&d_msg);
            }
        }
        //!\part :judge total insert row
        if(insert_row_count != add_list_count){
            str.clear();
            std::sprintf((char*)str.data()," 数据库插入{} 条，总共 {} 条；插入错误 {} 条",
                         insert_row_count, add_list_count, add_list_count - insert_row_count);
            spdlog::error(" {} ",str);
            js_m["insert_row"] = str;

            err_msg.clear();
            for (it = insert_error_map.begin(); it != insert_error_map.end();it++)
            {
                err_msg.append(std::to_string(it->first));
                err_msg.append(it->second);
            }
            spdlog::error("{}",err_msg); //!\note 可以把此字符串写入文件,或上传至服务器
        }else{
            js_m["insert_row"] = "okay";
        }
        /** @note monitor file content saved */
        //js_m["table_type"] = 2 ;        ///> input edit table
        js_m["row"] = 0 ;
        js_m["mid"] = js[j_data][j_edt_list][0][key_id];
        context = js_m.dump();
        monitor_file_update(m_file,context);
        return 0; //! return run success

    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -6;
    }
    return -7;
}


/**
 * @brief version_infor::download_edit_list_lib
 * @param d_name : json file
 * @param m_file : monitor row number
 * @return       : 0 is download okay, > 0 list is empty, < 0  is error
 */
int version_infor::download_edit_list_lib(const char * d_name, const char * m_file)
{

    std::string context,mContext;
    json js, js_m ;
    std::ifstream input_file(d_name);
    if(!input_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, d_name);
        input_file.close();
        return -1;
    }
    input_file.seekg(0,std::ios::end);
    int size = input_file.tellg();
    input_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();
    if (size < 10 ){
        spdlog::error(" line: {} file {} content too few, abort exit..", __LINE__, d_name);
        return -2;
    }

    std::ifstream monitor_file(m_file);
    if(!monitor_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, m_file);
        monitor_file.close();
        return -3;
    }
    monitor_file.seekg(0,std::ios::end);
    size = monitor_file.tellg();
    monitor_file.seekg(0,std::ios::beg);
    mContext.assign(std::istreambuf_iterator<char>(monitor_file),std::istreambuf_iterator<char>());
    monitor_file.close();
    if(size < 1 ){
        spdlog::error(" line:{}, file {} content is empty, abort exit..", __LINE__, m_file);
        return -4;
    }

    std::map<uint8_t, std::string> insert_error_map;
    std::map<uint8_t, std::string>::iterator it;
    std::string err_msg;
    std::string str,tableName;
    int type;
    int ret;
    int insert_row_count = 0 ;
    cmd_update_progress msg;
    cmd_download_progress d_msg;
    memset((char*)&msg,0,sizeof(msg));

    try{
        js = nlohmann::json::parse(context);
        js_m = nlohmann::json::parse(mContext);
//        type = js_m["table_type"];
//        if( type != 2){
//            spdlog::error(" line:{}, table type {} don't mach add_list, abort exit..", __LINE__, type);
//            return -5;
//        }

        int edt_list_count = js[j_data][j_edt_list].size();
        if( edt_list_count == 0 ){
            spdlog::info("file {} content edit_list[] is empty,exit..",m_file);
            return 1;
        }

        int i = js_m["row"];            //! \note :monitor row pass to i = row
        if (i > edt_list_count){
            spdlog::info("line: {}, file {} content row is ERROR,abort exit..", __LINE__, m_file);
            return 2;
        }

        for( ; i < edt_list_count; i++)
        {
            //! \part 2: role scenery get table name
            int mid = js[j_data][j_edt_list][i][key_id];
            std::string name = js[j_data][j_edt_list][i][file_name];
            type= js[j_data][j_edt_list][i][role_scenery];
            switch (type) {
            case 1:
                tableName = "NORMAL_ACTION";
                break;
            case 2:
                tableName = "SLEEP_ACTION";
                break;
            case 3:
                tableName = "EVENT_ACTION";
                break;
            default:
                tableName.clear();
                break;
            }

            //! \part 3: insert row to table
            if (tableName.size() == 0){
                err_msg = "下发数据的表类型错误,联系运营人员,文件名称:";
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -3;
            }

            ret = table_update_row( (char*)tableName.data(), js, (char*)j_edt_list, i );
            if ( ret < 0 ){
                err_msg = "数据库更新错误,错误代码： ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -4;
            }

            //!\part 4.1 : create download resource path
            char path[128];
            char cmd[128];
            ret = js[j_data][j_edt_list][i][key_id];
            memset(path,0,sizeof(path));
            sprintf(path,"%s/%d/",LIBRARY_BASE_PATH,ret);

            //!\note delete forlder content
            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"rm -r %s*",path);
            system(cmd);
            spdlog::debug(" run system call: {} ",cmd);

            ret = create_multi_dir(path);       /* create dir must be "%s/%d/" format */
            if(ret != 0)
            {
                err_msg = "create forld error,code: ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -5;
            }

            type = js[j_data][j_edt_list][i][f_lib_para];
            str = js[j_data][j_edt_list][i][url_path];
            int cnt = 0 ;
            //! \part 4.2: get download brain firmware url
            do{
                ret = download_library_file(path,(char*)str.data(),type);
                cnt +=1;
            }while( cnt < 5 && ret != 0);

            if( ( cnt >= 5 ) && ( ret != 0 ) )
            {
                //!\part 5.1:send  downlaod failure
                d_msg.device_type = 1; ///> brain
                d_msg.progress = 0;
                d_msg.result = DOWNLOAD_FAILUR;                        //failer
                cmd_download_progress_ui(&d_msg);

                err_msg = "download file failure,code: ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
            }else{
                insert_row_count += 1;
                /** @note monitor file content saved */
                js_m["row"] = i ;
                js_m["mid"] = js[j_data][j_add_list][i][key_id];
                std::string s = js_m.dump();
                monitor_file_update(m_file,s);

                //!\part 5.2:send downlaod progress
                d_msg.device_type = 1;                  ///> brain
                d_msg.progress = 100;
                d_msg.result = DOWNLOAD_SUCCESS;
                cmd_download_progress_ui(&d_msg);
            }
        }
        //!\part :judge total insert row
        if(insert_row_count != edt_list_count){
            str.clear();
            std::sprintf((char*)str.data()," 数据库更新 {} 条，总共 {} 条；更新错误 {} 条",
                         insert_row_count, edt_list_count, edt_list_count - insert_row_count);
            spdlog::error(" {} ",str);
            js_m["edit_row"] = str;

            err_msg.clear();
            for (it = insert_error_map.begin(); it != insert_error_map.end();it++)
            {
                err_msg.append(std::to_string(it->first));
                err_msg.append(it->second);
            }
            spdlog::error("{}",err_msg); //!\note 可以把此字符串写入文件,或上传至服务器           

        }else{
            js_m["edit_row"] = "okay";
        }
        /** @note monitor file content saved */
        //js_m["table_type"] = 3 ;        ///> input delete table
        js_m["row"] = 0 ;
        js_m["mid"] = js[j_data][j_del_list][0][key_id];
        context = js_m.dump();
        monitor_file_update(m_file,context);
        return 0; //! return run success

    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -6;
    }
    return -7;
}

/**
 * @brief version_infor::download_delete_list_lib
 * @param fName :json file
 * @param row       :monitor row number
 * @return          :0 is download okay, > 0 list is empty, < 0  is error
 */
int version_infor::download_delete_list_lib(const char * d_name, const char * m_file)
{
    std::string context,mContext;
    json js, js_m;
    std::ifstream input_file(d_name);
    if(!input_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, d_name);
        return -1;
    }
    input_file.seekg(0,std::ios::end);
    int size = input_file.tellg();
    input_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();
    if (size < 10 ){
        spdlog::error(" line: {} file {} content too few, abort exit..", __LINE__, d_name);
        return -2;
    }

    std::ifstream monitor_file(m_file);
    if(!monitor_file.is_open()){
        spdlog::error("line:{} open {} faile ", __LINE__, m_file);
        monitor_file.close();
        return -3;
    }
    monitor_file.seekg(0,std::ios::end);
    size = monitor_file.tellg();
    monitor_file.seekg(0,std::ios::beg);
    mContext.assign(std::istreambuf_iterator<char>(monitor_file),std::istreambuf_iterator<char>());
    monitor_file.close();
    if(size < 1 ){
        spdlog::error(" line:{}, file {} content is empty, abort exit..", __LINE__, m_file);
        return -4;
    }

    std::map<uint8_t, std::string> insert_error_map;
    std::map<uint8_t, std::string>::iterator it;
    std::string err_msg;
    std::string str,tableName;
    int type;
    int ret;
    int insert_row_count = 0 ;
    cmd_update_progress msg;
    cmd_download_progress d_msg;
    memset((char*)&msg,0,sizeof(msg));

    try{
        js = nlohmann::json::parse(context);
        js_m = nlohmann::json::parse(mContext);
//        type = js_m["table_type"];
//        if( type != 3){
//            spdlog::error(" line:{}, table type {} don't mach det_list, abort exit..", __LINE__, type);
//            return -5;
//        }

        int del_list_count = js[j_data][j_del_list].size();
        if( del_list_count == 0 ){
            spdlog::info("file {} content delete_list[] is empty,exit..",m_file);
            return 1;
        }
        int i = js_m["row"];                //! \note :monitor row pass to i = row
        if (i > del_list_count){
            spdlog::info("line: {}, file {} content row is ERROR,abort exit..", __LINE__, m_file);
            return 2;
        }

        for( ; i < del_list_count; i++)
        {
            //! \part 2: role scenery get table name
            int mid = js[j_data][j_del_list][i][key_id];
            std::string name = js[j_data][j_del_list][i][file_name];
            type= js[j_data][j_del_list][i][role_scenery];
            switch (type) {
            case 1:
                tableName = "NORMAL_ACTION";
                break;
            case 2:
                tableName = "SLEEP_ACTION";
                break;
            case 3:
                tableName = "EVENT_ACTION";
                break;
            default:
                tableName.clear();
                break;
            }

            //! \part 3: delete row to table
            if (tableName.size() == 0){
                err_msg = "下发数据的表类型错误,联系运营人员,文件名称:";
                err_msg.append(name);
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                it = insert_error_map.end();
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -3;
            }
            ret = table_delete_row( (char*)tableName.data(), js, (char*)j_del_list, i );
            if ( ret < 0 ){
                err_msg = "数据库删除错误,错误代码： ";
                err_msg.append(std::to_string(ret));
                err_msg.append(" file_name:");
                err_msg.append(name);
                it = insert_error_map.end();
                insert_error_map.insert(std::pair<int,std::string>(mid,err_msg));
                spdlog::error(" 错误文件编号: {}, 描述:{} " ,it->first, it->second);
                continue;
                //return -4;
            }

            //!\part 4.1: create download resource path
            char path[128];
            char cmd[128];
            ret = js[j_data][j_del_list][i][key_id];
            memset(path,0,sizeof(path));
            sprintf(path,"%s/%d/",LIBRARY_BASE_PATH,ret);

            //!\part 4.2: delete forlder content
            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"rm -r %s*",path);
            system(cmd);
            spdlog::debug(" run system call: {} ",cmd);

            insert_row_count += 1;
            /** @note monitor file content saved */
            js_m["row"] = i ;
            js_m["mid"] = js[j_data][j_del_list][i][key_id];
            std::string s = js_m.dump();
            monitor_file_update(m_file,s);
        }

        //!\part :judge total insert row
        if(insert_row_count != del_list_count){
            str.clear();
            std::sprintf((char*)str.data()," 数据库删除 {} 条，总共 {} 条；删除错误{} 条",
                         insert_row_count, del_list_count, del_list_count - insert_row_count);
            spdlog::error(" {} ", str);
            js_m["del_row"] = str;

            err_msg.clear();
            err_msg.append("数据库删除错误信息: ");
            for (it = insert_error_map.begin(); it != insert_error_map.end();it++)
            {
                err_msg.append(std::to_string(it->first));
                err_msg.append(it->second);
            }
            spdlog::error("{}",err_msg); //!\note 可以把此字符串写入文件,或上传至服务器

        }else{
            js_m["del_row"] = "okay";
        }
        //!\part 5.2: send downlaod progress
        d_msg.progress = 100;
        d_msg.device_type = 1; ///> brain
        d_msg.result = DOWNLOAD_SUCCESS;                            //success
        cmd_download_progress_ui(&d_msg);
        /** @note monitor file content saved */
        //js_m["table_type"] = 1 ;        ///> input delete table
        js_m["row"] = 0 ;
        js_m["mid"] = 0 ;
        context = js_m.dump();
        monitor_file_update(m_file,context);
        return 0; //! return run success

    }catch(std::exception & e){
        spdlog::error("{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -6;
    }
    return -7;
}

/**
* @brief version_infor::compare_firmware_and_library
* @param logic_type
* @param firmware_v
* @param library_v
* @return
*/
int version_infor::compare_firmware_and_library(uint8_t logic_type, uint32_t firmware_v, uint32_t library_v)
{

   char version[20];
   memset(version,0,sizeof(version));
   char fileName[64];
   memset(fileName,0,sizeof(fileName));
   int ret = 0;
   uint8_t V[4];
   //!little mode
   V[0] = (uint8_t)(firmware_v>>24);
   V[1] = (uint8_t)(firmware_v>>16);
   V[2] = (uint8_t)(firmware_v>>8);
   V[3] = (uint8_t)(firmware_v>>0);
   std::sprintf(version,"%d.%d.%d.%d",V[0],V[1],V[2],V[3]);
   int type = logic_to_upgrade_type(logic_type);

   ret = find_comment_firmware_name(type,version,fileName);
   if(ret == 0 ){
       spdlog::debug("firmware_v:{}, upgrade type: {} ",version,type);
       return 0;
   }

   memset(version,0,sizeof(version));
   //!little mode
   V[0] = (uint8_t)(library_v>>24);
   V[1] = (uint8_t)(library_v>>16);
   V[2] = (uint8_t)(library_v>>8);
   V[3] = (uint8_t)(library_v>>0);
   std::sprintf(version,"%d.%d.%d.%d",V[0],V[1],V[2],V[3]);
   std::string ver = get_catalog_file_library_version(device_catalog_file);
   ret = strcmp(version,(char*)ver.c_str());
   if(ret !=0){
       spdlog::debug("library_v:{}, upgrade type: {}",version,type);
       return 0;
   }
   return 1;
}

/*
 *
 *
*/
 version_infor::version_infor(void)
{
    spdlog::debug(" version manager thread startup ... \n");

}
/*
 *
 *
*/
version_infor::~version_infor(void)
{

}





















