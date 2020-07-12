
//user file
#include "ipc_with_ui.hpp"
#include "queue_source_enum.h"
#include "robot.hpp"

///downlaod
#include "down_load_file.hpp"
#include "infor_unpacket.hpp"
#include "version_infor.h"
#include "update_version.hpp"
#include "version_infor.h"
#include "forlder_util.h"
//
using json = nlohmann::json;
using namespace std;

//!
//! \brief cloud url address
const char *FILE_PATH_BASE = "/userdata/comment/";
const char *GET_URL_BASE = "http://neuro.coolphper.com/api/firmware/firmwareApi";
const char *FILE_PATH_NAME= "/userdata/comment/comment.jscm";
const char *FIRMWARE_CONFIG_PATH_FILE = "/userdata/comment/comment.jscm";

BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_from_core;
BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_to_core;
BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_firmware_rx_from_serial_queue;
BlockingConcurrentQueue<std::vector<uint8_t>> rly_send_to_ui_queue;
version_infor infor;

extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;
extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_serial_queue;
extern Robot    robot;

//user define marco
#define	PROCESS_NAME	"brain-robot-app"
#define DEBUG_FIRMWARE_FILE "/userdata/comment/4/sub.bin"  //"/userdata/lpc54113.bin"
#define UPDATE_FIRMWAR_M_Q_NAME    "/UPDATE_FIRMWAR_M_Q"

#define DEBUG_FIRMWARE_FILE_ENABLE
#define	BUF_MAX_SIZE	2048
#define TO_UI_MSG_MAX_SIZE 4096


//debug test marco
#define JOINT_FIRMWARE_FILE "/userdata/comment/4/Joint.bin"
#define SKELETON_FIRMWARE_FILE "/userdata/comment/4/Skeleton.bin"
#define WHEEL_FIRMWARE_FILE "/userdata/comment/4/Wheel.bin"
#define SUBLCD_FIRMWARE_FILE "/userdata/comment/4/sub.bin"

/*
 *  name    :upgrade_comment_firmware()
*/

void Robot::upgrade_comment_firmware(void)
{
    //std::map<uint8_t, Unit*>::iterator iter;
    uint32_t ret;
    std::map<int, Unit*>::iterator iter;
    char firmware_file_name[128];
    uint32_t Version ;
    int32_t upgrade_result = 0;
  //   for(iter=raw_structure_map.begin(); iter != raw_structure_map.end(); iter++)
            for(iter=logic_strucutre_map.begin(); iter != logic_strucutre_map.end(); iter++)
              {
                  //uint32_t Version = (iter->second)->get_firmware_version();
                  uint8_t type = (iter->second)->get_type();
                  uint32_t deviceNumber = (iter->second)->get_num();
                  memset(firmware_file_name,0,sizeof(firmware_file_name));
                  if(type == 0){ //brain next..
                      spdlog::debug(">>>>>  check deviceNumber: {}, type: {} ,debug next..",deviceNumber, type);
                      upgrade_result += 1;
                      continue;
                   }
  /** debug start */
//                  if(deviceNumber == 1){ //skip first number, next..
//                      spdlog::debug(">>>>>  check deviceNumber: {}, type: {},debug next..",deviceNumber, type);
//                      continue;
//                   }
//                  if(deviceNumber == 2){ //skip second number, next..
//                      spdlog::debug(">>>>>  check deviceNumber: {}, type: {},debug next..",deviceNumber, type);
//                      continue;
//                   }

//                    switch(type){
//                    case TYPE_JOINT:
//                        memcpy(firmware_file_name,JOINT_FIRMWARE_FILE,sizeof(JOINT_FIRMWARE_FILE) );
//                        break;
//                    case TYPE_SKELETON:
//                        memcpy(firmware_file_name,SKELETON_FIRMWARE_FILE,sizeof(SKELETON_FIRMWARE_FILE) );
//                        break;
//                    case TYPE_WHEEL:
//                        memcpy(firmware_file_name,WHEEL_FIRMWARE_FILE,sizeof(WHEEL_FIRMWARE_FILE) );
//                        break;
//                    default:
//                        memset(firmware_file_name,0,sizeof(firmware_file_name));
//                        spdlog::error("firmware_file_name is null !!");
//                        break;
//                    }
//                spdlog::debug(">>>>>  check deviceNumber: {}, type: {},debug device ..",deviceNumber, type);
/** debug end */

                //part 1: mach local version and running version
                 Version = this->upgrade_get_firmwart_version(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
                                                                         upgrade_firmware_rx_from_serial_queue);
                 if(Version == 0xffffffff)
                 {
                     spdlog::error(">>>>>  deviceNumber: {}, type: {} ,version is error {} next..",deviceNumber, type,Version);
                     continue;
                 }
/** debug start */
//                 this->upgrade_send_firmwart_abandon(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
//                                                     upgrade_firmware_rx_from_serial_queue);
//                 this->contral_joint_led_status(SOURCE_STRUCTURE_UPDATER_THREAD,deviceNumber,
//                                                 1, 3, 55,1,upgrade_firmware_rx_from_serial_queue);
//                 ret = this->upgrade_send_firmware_file_name(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
//                                                        firmware_file_name, upgrade_firmware_rx_from_serial_queue);
//                 if(ret != 0 )
//                     continue;
//                  std::this_thread::sleep_for(std::chrono::milliseconds(50));
//                  this->upgrade_loop_firmware_file(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
//                                                        firmware_file_name, upgrade_firmware_rx_from_serial_queue);
//                  this->upgrade_send_firmware_file(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
//                                                        firmware_file_name, upgrade_firmware_rx_from_serial_queue);
/** debug end */
                  //part 2: normalize version and type,check version equ

                   ret = infor.normalize_device_version_type(type,Version,firmware_file_name);
                  if(ret == 0)  //! search valid comment , upgrade it.
                  {

                      ret = this->contral_joint_led_status(SOURCE_STRUCTURE_UPDATER_THREAD,deviceNumber,
                                                      1, 3, 155,1,upgrade_firmware_rx_from_serial_queue);
                      if(ret != 0 )
                      {//! upgrade comment failure
                          spdlog::error(" deviceNumber: {}, type: {} ,device non response next..",deviceNumber, type);
                          std::this_thread::sleep_for(std::chrono::milliseconds(50));
                          continue;
                      }
                      //part 3:upgrade firmwave file
                      this->upgrade_send_firmwart_abandon(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
                                                          upgrade_firmware_rx_from_serial_queue);
                      this->upgrade_send_firmware_file_name(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
                                                            firmware_file_name/*DEBUG_FIRMWARE_FILE*/, upgrade_firmware_rx_from_serial_queue);
                      this->upgrade_send_firmware_file(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
                                                            firmware_file_name/*DEBUG_FIRMWARE_FILE*/, upgrade_firmware_rx_from_serial_queue);
//                      this->upgrade_send_firmwart_reset(SOURCE_STRUCTURE_UPDATER_THREAD,deviceNumber,
//                                                        upgrade_firmware_rx_from_serial_queue);

                      this->contral_joint_led_status(SOURCE_STRUCTURE_UPDATER_THREAD,deviceNumber,
                                                      1, 1, 55,1,upgrade_firmware_rx_from_serial_queue);

                      std::this_thread::sleep_for(std::chrono::seconds(5)); /* wait 5s structer upgrade */

                  }
                  else{
                      spdlog::debug(" deviceNumber: {}, Version: {}, type: {} , don't upgrade..",deviceNumber, Version, type);
                  }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
              }

       /* check sub lcd firmware */
//         spdlog::debug("   >>>>>  check sub lcd firmware version ");
//         Version =  this->get_54113_lcd_motor_version();

//         //part 4: normalize version and type,check version equ
//        ret = infor.normalize_device_version_type(/*TYPE_SUB_LCD*/ 8 ,Version,firmware_file_name);
//        if(ret == 0){
//         //part 5:upgrade firmwave file
//         spdlog::debug("   ...... upgrade sub lcd firmware ..... ");
//         this->send_54113_lcd_motor_file(SUBLCD_FIRMWARE_FILE,UPGRADE_SUB_LCD_TYPE);
//         this->send_54113_lcd_motor_reset();
//        }

        cmd_core_to_ui_reboot msg;
        memset((char*)&msg, 0, sizeof(msg));
        if (upgrade_result != 0 )
            msg.result = 2;
        else {
            msg.result = 1;
        }
        cmd_upgrade_reboot_notify_ui(&msg);
}
/**
 * @name  :upgrade_comment_firmware_req
 * @brief :request firmware upgrade
 *
*/

void Robot::upgrade_comment_firmware_req(void)
{

        std::map<uint8_t, std::string> version_map;
        std::map<uint8_t, std::string>::iterator it;
        std::map<int, Unit*>::iterator iter;
        uint32_t Version;
        //std::string    Ver_str;
        char Ver_str[18];
        uint8_t type;
        uint32_t deviceNumber;
        for(iter=logic_strucutre_map.begin();iter!=logic_strucutre_map.end();iter++)
        {
            type = (iter->second)->get_type();
            deviceNumber = (iter->second)->get_num();
            it = version_map.find(type);
            if(it != version_map.end()){
                                continue;
            }
            Version = this->upgrade_get_firmwart_version(SOURCE_STRUCTURE_UPDATER_THREAD, deviceNumber,
                                          upgrade_firmware_rx_from_serial_queue);
            if(Version == 0xffffffff)
            {
                spdlog::debug(">>>>>  deviceNumber: {}, type: {} ,version is error {} next..",deviceNumber, type,Version);
                continue;
            }
            memset(Ver_str,'\0',sizeof(Ver_str));
            std::sprintf((char*)Ver_str,"%d.%d.%d.%d",
                        (uint8_t)(Version>>24),(uint8_t)(Version>>16),
                         (uint8_t)(Version>>8),(uint8_t)(Version>>0));
            //version_map[type] = Ver_str;
            version_map.insert(std::pair<int, std::string>(type,Ver_str));
          }

        //! sub lcd version
//        Version =  this->get_54113_lcd_motor_version();
//        if(Version == 0xffffffff)
//        {
//            spdlog::debug(">>>>>  deviceNumber: {}, type: {} ,version is error {} next..",deviceNumber, type,Version);
//            return;
//        }
//        memset(Ver_str,'\0',sizeof(Ver_str));
//        std::sprintf((char*)Ver_str,"%d.%d.%d.%d",
//                    (uint8_t)(Version>>24),(uint8_t)(Version>>16),
//                     (uint8_t)(Version>>8),(uint8_t)(Version>>0));
//        version_map.insert(std::pair<int, std::string>(8,Ver_str));//! 8 is sub lcd type

        for(it = version_map.begin();it != version_map.end();it++)
            spdlog::debug("version_map_1:{},version_map_2:{}",it->first,it->second);
        infor.upgrade_comment_request_ui(FIRMWARE_CONFIG_PATH_FILE,version_map);

}

/**
 * @name     : update_firmware_entry()
 *
*/

int update_firmware_entry(void)
{
    int ret;
    /* debug start*/
    const char * post_sava_path = "/userdata/comment/post_in.jscm";
    const char * post_url_base = "http://neuro.coolphper.com/api/ku/kuapi";
    const char * post_field = "{\"version\":\"0.2\",\"page\":1,\"length\":100}";
    bool b;
    b = postUrl((char *)post_sava_path, (char*)post_url_base, (char*)post_field);
    if (!b) {
        spdlog::error("post url {} respose error..",post_url_base);
    }
    ret = infor.post_json_to_file(post_sava_path);
    if (ret < 0 ){
        spdlog::error(" {} download_library_request_ui error...",post_sava_path);
    }
    if (ret == 1 ){
        //!\note library list is empty , nothing do
        cmd_version_unchange_notify_ui();
     }
     infor.download_library_request_ui(post_sava_path);
    /* end */

    uint8_t buf[BUF_MAX_SIZE];
   // uint16_t    get_version_count = 0;
    std::vector<uint8_t> rx_buf;
    infor.get_brain_version();

    while (1) {

        upgrade_queue_from_core.wait_dequeue_timed(rx_buf,
                                                   std::chrono::milliseconds(500));
        //spdlog::debug("reci from core msg:{}",spdlog::to_hex(rx_buf));
        if(rx_buf.size() == 0)
        {
            //spdlog::error("reci size = 0 ,no from core msg");
            continue;
        }
        //spdlog::debug("recv from core msg:{}",spdlog::to_hex(rx_buf));
        uint16_t i=0;
        uint16_t len=rx_buf.size();
        for(;i<len;i++)
        {
            buf[i]=rx_buf.at(i);
            if(i > BUF_MAX_SIZE)
            	{
            		spdlog::debug("data over 1024:",spdlog::to_hex(rx_buf));
                break;
              }
        }
//        std::vector<uint8_t> tmp(buf, buf + len);
//        spdlog::debug("data:{} , len:{}",spdlog::to_hex(tmp),len);
        switch(*(uint16_t*)buf)
        {

        case CMD_UI_VERSION_FROM_CORE:

            break;

        case CMD_LINKNET_NOTIFY_FROM_CORE:
        {
            /*action: 1,download comment firmware file,and create forlder
             *        2,send cmd_update_request command*/

            //spdlog::debug("{}:line {}.. ",__FILE__,__LINE__);
            //!part 1: download comment firmware file,and create forlder
            create_multi_dir(FILE_PATH_BASE);
            bool sb;
            sb = getUrl((char*)FILE_PATH_NAME,(char*)GET_URL_BASE);
            if(sb != true){
                spdlog::error("down load {} file done failer...",FILE_PATH_NAME);
                break;
            }
            ret = infor.get_json_to_file(FILE_PATH_NAME);
            if(ret != 0 ){
                spdlog::error(" {} file parse failer...",FILE_PATH_NAME);
                break;
            }
            //!part 2: send cmd_download_request command
            infor.download_brain_request_ui(FILE_PATH_NAME);
        }
            break;
        case CMD_DOWNLOAD_CONFIRM_FROM_CORE:
        {
            mq_msg_format *msg = (mq_msg_format*)buf;
            if( (msg->data[2] <= 3)&&(msg->data[3] == AGREE_UPDATE) )
            {
                //brain download process,and send upgrade brain request
                spdlog::info("user select  download firmware file...");
                infor.brain_download_firmware(FILE_PATH_NAME);
                infor.upgrade_brain_request_ui(FILE_PATH_NAME);
            }
            else {
                spdlog::debug("{}:line {}: type error|| give up upgrade..",__FILE__,__LINE__);
            }

        }
            break;

        case CMD_UPGRADE_CONFIRM_FROM_CORE:
        {
            /*action:  1, update brain firmware and application;
             *         2, set comment user agree */
            //part 1:update brain firmware and application;

            mq_msg_format *msg = (mq_msg_format*)buf;
            if( (msg->data[2] <= 3)&&(msg->data[3] == AGREE_UPDATE) )
            {
                //brain update process
                infor.brain_upgrade_firmware(FILE_PATH_NAME);
                spdlog::debug("user select  update BRAIN system file...");
            }
            else
                spdlog::debug("device type is error,don't upgrade firmware ...");
        }
            break;

        case CMD_UPGRADE_COMMENT_CONFIRM_FROM_CORE:
        {
            /*
             * recive upgrade comment response from ui or structure change event
             * todo: upgrade_comment_firmware()
             *
             */
            robot.upgrade_comment_firmware();

//            std::vector<uint8_t> tx_buf;
//            tx_buf.push_back(0x00);
//            tx_buf.push_back(0x01);
//            tx_buf.push_back(0x55);
//            tx_buf.push_back(0xaa);
//            upgrade_queue_to_core.enqueue(tx_buf);
//            spdlog::info("user select upgrade comment confirm infor to...");
        }
            break;

        default:
            spdlog::debug("command is invalid,msg:{}",spdlog::to_hex(rx_buf));
            break;
        }
        rx_buf.clear();
    }
}

/*
 * breif    @ send upgrade comment request
 *
*/

void rut_send_message_to_ui(mqd_t & mq, struct mq_attr & attr, char * message_buf, uint16_t msg_type, uint16_t index, uint16_t error_code, uint16_t len_of_msg)
{

    *(uint16_t *)(message_buf) = msg_type;
    *(uint16_t *)(message_buf + 2) = index;  // index
    *(uint16_t *)(message_buf + 4) = error_code;  // error code
    *(uint16_t *)(message_buf + 6) = len_of_msg;  // length of message
    mq_getattr(mq, &attr);
    if (49 == attr.mq_curmsgs) {
        spdlog::error("can not send message to UI, message queue is full");
        return;
    }
    spdlog::debug("send info to UI: {}", spdlog::to_hex(message_buf, message_buf + 8 + len_of_msg));
    mq_send(mq, message_buf, 8 + len_of_msg, 0);
}


void rut_tx_to_ui_thread_entry(void)
{
    mqd_t mq_tx_to_ui;
    char command_to_ui[1024];
    struct mq_attr attr;
    attr.mq_maxmsg = 50;
    attr.mq_msgsize = 1024;
    std::vector<uint8_t> rx_buf;

    do {
        mq_tx_to_ui = mq_open(RUT_MQ_TX_TO_UI, O_WRONLY | O_CREAT, 0666, &attr);
        if (mq_tx_to_ui < 0) {
            spdlog::error("failed to open message queue to UI: {}, error: {}, retrying...",
                    mq_tx_to_ui, strerror(errno));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (mq_tx_to_ui < 0);

    while (true) {

        rly_send_to_ui_queue.wait_dequeue(rx_buf);

        spdlog::debug("send to UI queue message: {}", spdlog::to_hex(rx_buf));

        if (0 == rx_buf.size()) {
            spdlog::error("no message dequeued");
            continue;
        }

        switch(rx_buf.at(0)) {

            case INFO_DEBUG_FUCTION:
            {
                uint16_t command = CMD_LINKNET_NOTIFY_FROM_CORE;
                uint16_t len = rx_buf.size()-2;
                memset(command_to_ui,0,sizeof(command_to_ui));
                memcpy((char*)(command_to_ui + 8),(char*)(rx_buf.data()+2), len);
                rut_send_message_to_ui(mq_tx_to_ui, attr, command_to_ui, command, 0x0000, 0x0000, len);
                }
                break;
            case INFO_MYSELF_UPGRADE_COMMENT_REQ:
            {
                robot.upgrade_comment_firmware_req();
            }
            break;
            case INFO_MYSELF_UPGRADE_COMMENT:
            {
                uint16_t command = CMD_UPGRADE_COMMENT_CONFIRM_FROM_CORE;
                uint16_t len = rx_buf.size()-2;
                memset(command_to_ui,0,sizeof(command_to_ui));
                memcpy((char*)(command_to_ui + 8),(char*)(rx_buf.data()+2), len);
                rut_send_message_to_ui(mq_tx_to_ui, attr, command_to_ui, command, 0x0000, 0x0000, len);
                }
            break;
           //-- debug command -----
            default:
                spdlog::debug("unknown type, can not send to UI");
                break;
        }
        rx_buf.clear();
    }
}

