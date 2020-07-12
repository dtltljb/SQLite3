#ifndef VERSION_INFOR_H
#define VERSION_INFOR_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
/// thirty libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "nlohmann/json.hpp"
#include "blockingconcurrentqueue.h"

/// using name space
using json = nlohmann::json;
using namespace moodycamel;     //moodycamel::BlockingConcurrentQueue


#include "infor_unpacket.hpp"
#define PROCESS_BAS_PATH    "/usr/sbin"

#define MAX_VERSON_TOTAL    55
#define FILE_PATH           "/userdata/comment"
#define SYS_UPDATE_CMD      "update ota"
#define VERSION_BASE_PATH   "/userdata/comment"


extern const char *SYSTEM_VERSION_PATH;

typedef enum{
    UPGRADE_BRAIN_IMAGE_TYPE = 1,
    UPGRADE_BRAIN_UI_TYPE =  2,
    UPGRADE_BRAIN_CORE_TYPE =    3,
    UPGRADE_SUB_LCD_TYPE =   4,
    UPGRADE_JOINT_TYPE  =   5,
    UPGRADE_SKELETON_TYPE  =   6,
    UPGRADE_WHEEL_TYPE  =   7,
    UPGRADE_CLAW_TYPE  =   8,
    UPGRADE_CUPULE_TYPE  =   9,
    UPGRADE_DISTOMAT_TYPE = 10,
}upgrade_device_type;

class version_infor
{
public:
    version_infor(void);
    ~version_infor(void);
    void find_dir_key_word(const char *key,const char * path,std::vector<std::string> &list);
    void brain_upgrade_firmware(const char * file_name);
    int brain_download_firmware(const char * file_name);
    int monitor_file_update(const char * f_name, std::string &j_str);
    int get_post_field_content(const char *c_file, std::string &j_interface, std::string &field);
    int get_post_interface_time(const char *p_file, int64_t *time);


    /**
     * @brief version_infor::download_add_list_lib
     * @param d_name :download library file
     * @param m_file :monitor progress file
     * @return       :0 is download okay, > 0 list is empty, < 0  is error
     */
    int download_add_list_lib(const char * d_name, const char * m_file);

    /**
     * @brief version_infor::download_edit_list_lib
     * @param fName :json file
     * @param row       :monitor progress row number
     * @return          :0 is download okay, > 0 list is empty, < 0  is error
     */
    int download_edit_list_lib(const char * d_name, const char *m_file);

    /**
     * @brief version_infor::download_delete_list_lib
     * @param fName :json file
     * @param row       :monitor progress row number
     * @return          :0 is download okay, > 0 list is empty, < 0  is error
     */
    int download_delete_list_lib(const char * d_name, const char *m_file);

    int get_json_to_file(const std::string & file_name);
    int post_json_to_file(const std::string & file_name);

    void upgrade_brain_request_ui(const std::string & file_name);
    void upgrade_comment_request_ui(const std::string & file_name,
                                    std::map<uint8_t, std::__cxx11::string> &version_map);
    int upgrade_comment_library_version(const char *file, std::string & version);

    int download_brain_request_ui(const std::string & file_name);
    int download_library_request_ui(const std::string & file_name);

    int logic_to_upgrade_type(int logic_type);
    int upgrade_to_logic_type(int upgrade_type);
    int normalize_device_version_type(uint8_t upgrade_type, uint32_t ver_num, char *output_firmware_file_name);

    void save_ui_version(char *pVer, uint8_t len );
    void get_brain_version(void);
    void read_brain_ui_version(void);
    int get_local_total_version(cmd_core_to_ui_total_version * msg,uint8_t *count);

    std::string get_ui_version( void );
    std::string get_sys_version( void );
    std::string get_core_version( void );

    int get_brain_config_info(const char * c_file, std::string &info);
    int set_brain_config_infor(const char *c_file, std::string &j_str);
    /**
    * @brief version_infor::compare_firmware_and_library
    * @param logic_type
    * @param firmware_v
    * @param library_v
    * @return
    */
    int compare_firmware_and_library(uint8_t logic_type, uint32_t firmware_v, uint32_t library_v);

private:
    //brain_firmware_inter Version;
    std::string ui_ver;
    std::string core_ver;
    std::string sys_ver;
    std::string user_ver;        /* user version send ui display */

};

#endif // VERSIONINFOR_H
