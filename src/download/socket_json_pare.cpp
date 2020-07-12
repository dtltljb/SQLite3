
/*
 * robot.cpp
 *
 *  Created on: 2019年3月4日
 *      Author: jinglong
 */


//user header
//#include "brain.hpp"
//#include "joint.h"
//#include "wheel.h"
//#include "skeleton.h"
//#include "claw.hpp"
//#include "cupule.hpp"
#include "robot.hpp"
#include "queue_source_enum.h"
//#include "robot.hpp"
//#include "structure_updater.h"
//#include "socket_libevent.h"
//#include "units_command.h"
#include "ipc_with_ui.hpp"
#include "socket_json_pare1.hpp"

using json = nlohmann::json;
using namespace moodycamel;

extern Robot robot;
extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;
extern BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_firmware_rx_from_serial_queue;




static void socket_response_to_app(struct bufferevent *event_socket,char *buf,int len)
{
    std::vector<uint8_t> msg;

     msg.clear();
     msg.push_back(0x09);    //command
     msg.push_back(0x00);

     msg.push_back(0x00);    //index
      msg.push_back(0x00);
     if(len > 1024){
        spdlog::debug("{},line: {} send data big length,throw.. ", __FILE__,__LINE__);
        len = 1024;
     }
     msg.push_back((uint8_t)len);    //size lowest byte,length =4 bytes
     msg.push_back((uint8_t)(len>>8));
     msg.push_back(0x00);
     msg.push_back(0x00);
     int i=0;
     for(;i< len;i++)
         msg.push_back(buf[i]);

     if(event_socket == NULL){
          spdlog::debug("{},line:{} bufferevent_write is empty,throw.. ", __FILE__,__LINE__);
          return ;
     }
     bufferevent_write(event_socket, &(msg[0]), msg.size());
     spdlog::debug(" bufferevent_write send:\n{} ",spdlog::to_hex(msg));
}

int socket_json_pare(std::string  & json_string, struct bufferevent * dev_socket)
{
        json j_list =
            {   {"jsonType",1},
                {"list",
                     {
                        {"mIndex",1},
                        {"version","1.01.23.66"},
                        {"build",1}
                     }

                }/*NOTE:object create mothed */
            };

    /*
     *  {"jsonType":101,"list":[{"build":1,"mIndex":3,"version":"0.0.0.0"}]}
*/


    auto json_parsed = json::parse(json_string);
    std::string list ="{\"list\":[{\"mIndex\":1,\"version\":\"1.01.23.66\"}]}";
    int jsonType;
    try
    {
        // try to write at a nonexisting key
       jsonType = json_parsed["jsonType"];
    }
    catch (json::type_error& e)
    {
        std::cout << e.what() << '\n';
    }
    if(jsonType == 0){
        spdlog::debug("{},line: {} jsonType is empty,throw.. ", __FILE__,__LINE__);
        return -1;
    }
    switch (jsonType) {
    case 1:
    {
        std::string device_name = json_parsed["deviceName"].get<std::string>();
        spdlog::debug("设备类型: {}", device_name);
        std::vector<uint8_t> send_buf;
        send_buf.push_back(INFO_MOBILE_APP_CONNECTED);
        std::vector<uint8_t> tmp(device_name.begin(), device_name.end());
        send_buf.insert(send_buf.end(), tmp.begin(), tmp.end());
        send_buf.push_back('\0');
        send_to_ui_queue.enqueue(send_buf);
    }
        break;
    case 101:
    {
        try{
        int dev_num = json_parsed["list"][0];

//        if(this->logic_strucutre_map.find(dev_num) != this->logic_strucutre_map.end())
//        {
//            int Version = this->upgrade_get_firmwart_version(SOURCE_STRUCTURE_UPDATER_THREAD, dev_num,
//                                                       upgrade_firmware_rx_from_serial_queue);

//            char str[32];
//            memset(str,0,sizeof(str));
//            std::sprintf(str,"%d.%d.%d.%d",
//                (uint8_t)(Version>>24),(uint8_t)(Version>>16),(uint8_t)(Version>>8),(uint8_t)(Version>>0));

//            std::string s = j_list["list"][0]["version"].get<std::string>();
//            j_list["jsonType"] = jsonType;
//            j_list["list"][0]["mIndex"] = dev_num;
//            j_list["list"][0]["version"] = str;
//            std::string o_str = j_list.dump();

//            socket_response_to_app(dev_socket,(char*)o_str.data(),o_str.size());
//            spdlog::debug("response list: {}  ", o_str);
//        }

        }catch(json::parse_error& e)
        {
            spdlog::error("{},\n >>>> {} json parse {} ",__FILE__,__LINE__, e.what() );
        }
    }
        break;

    default:
        spdlog::debug("{},line: {} jsonType= {} unknow,throw.. ", __FILE__,__LINE__,jsonType);
        break;
    }

    return 0;
}







