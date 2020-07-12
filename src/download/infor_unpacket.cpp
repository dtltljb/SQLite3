#include <vector>
#include <iostream>

//third libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "blockingconcurrentqueue.h"

//src
#include "ipc_with_ui.hpp"

//sub module
#include "infor_unpacket.hpp"


using namespace moodycamel;     //moodycamel::BlockingConcurrentQueue

extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;



/*unpacket recive message */

//void cmd_ui_version_resp(uint16_t length,char *buf)
//{
//    if( (length > 5)||(buf == NULL) )
//        return;
//    memcpy(brain_version.ui_ver,buf,length);
//        return;
//}


/* packet send information message*/
void cmd_update_progress_ui(cmd_update_progress *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_UPDATE_PROGRESS);
    send_buf.push_back(0x00);
    send_buf.push_back((uint8_t)msg->device_type);
    send_buf.push_back(msg->progress);
    send_buf.push_back(msg->result);
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_upgrade_comment_progress_ui(cmd_update_progress *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_UPGRADE_COMMENT_PROGRESS);
    send_buf.push_back(0x00);
    send_buf.push_back((uint8_t)msg->device_type);
    send_buf.push_back(msg->progress);
    send_buf.push_back(msg->result);
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_download_progress_ui(cmd_download_progress *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_DOWNLOAD_PROGRESS);
    send_buf.push_back(0x00);
    send_buf.push_back((uint8_t)msg->device_type);
    send_buf.push_back(msg->progress);
    send_buf.push_back(msg->result);
    send_to_ui_queue.enqueue(send_buf);
}


/**
  send upgrade comment okay notify to ui
*/
void cmd_upgrade_reboot_notify_ui(cmd_core_to_ui_reboot *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_TO_UI_NOTIFY_REBOOT);
    send_buf.push_back(0x00);
    send_buf.push_back(msg->result);
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_version_unchange_notify_ui(void)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_TO_UI_NOTIFY_VERSION_UNCHANGE);
    send_buf.push_back(0x00);
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_get_version_to_ui(cmd_ui_version *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_READ_UI_VERSION);
    send_buf.push_back(0x00);
    send_buf.push_back((uint8_t)msg->result);
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_connect_failure_notify_ui(void)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(0x5a);                   //!  debug user
    send_buf.push_back(0x00);
    send_to_ui_queue.enqueue(send_buf);
}


void cmd_download_request_to_ui(cmd_update_request *msg)
{
    std::vector<uint8_t> send_buf;
    send_buf.push_back(INFO_DOWNLOAD_REQUEST);
    send_buf.push_back(0x00);
    send_buf.push_back(msg->status);
    send_buf.push_back((uint8_t)msg->device_type);
    uint8_t i =0;
    for(;i< sizeof(msg->Ver);i++)
        send_buf.push_back(msg->Ver[i]);
    // length
    send_buf.push_back((uint8_t)msg->desc_length);
    send_buf.push_back((uint8_t)(msg->desc_length>>8));
    int cnt = 0;
    for(;cnt < msg->desc_length;cnt++)
    {
        send_buf.push_back(msg->desc[cnt]);
        if(cnt > 1024)
            break;
    }
    send_to_ui_queue.enqueue(send_buf);
}

void cmd_download_request_ui(cmd_update_request *msg)
{
        std::vector<uint8_t> send_buf;
        send_buf.push_back(INFO_DOWNLOAD_REQUEST);
        send_buf.push_back(0x00);
        send_buf.push_back(msg->status);
        send_buf.push_back((uint8_t)msg->device_type);
        uint8_t i =0;
        for(;i< sizeof(msg->Ver);i++)
            send_buf.push_back(msg->Ver[i]);

        // length
        send_buf.push_back((uint8_t)msg->desc_length);
        send_buf.push_back((uint8_t)(msg->desc_length>>8));
        int cnt = 0;
        for(;cnt < msg->desc_length;cnt++)
        {
            send_buf.push_back(msg->desc[cnt]);
            if(cnt > 1024)
                break;
        }
        send_to_ui_queue.enqueue(send_buf);
}

void cmd_upgrade_request_ui(cmd_update_request *msg)
{
        std::vector<uint8_t> send_buf;
        send_buf.push_back(INFO_UPGRADE_REQUEST);
        send_buf.push_back(0x00);
        send_buf.push_back(msg->status);
        send_buf.push_back((uint8_t)msg->device_type);

        uint8_t i =0;
        for(;i< sizeof(msg->Ver);i++)
            send_buf.push_back(msg->Ver[i]);

        // length
        send_buf.push_back((uint8_t)msg->desc_length);
        send_buf.push_back((uint8_t)(msg->desc_length>>8));
        int cnt = 0;
        for(;cnt < msg->desc_length;cnt++)
        {
            send_buf.push_back(msg->desc[cnt]);
            if(cnt > 1024)
                break;
        }
        send_to_ui_queue.enqueue(send_buf);
}


void cmd_upgrade_comment_request_ui(comment_upgrade_request *msg)
{
        std::vector<uint8_t> send_buf;
        send_buf.push_back(INFO_TO_UI_UPGRADE_COMMENT);
        send_buf.push_back(0x00);
        send_buf.push_back(msg->status);
        send_buf.push_back((uint8_t)msg->device_type);

        uint8_t i =0;
        for(;i< sizeof(msg->Ver);i++)
            send_buf.push_back(msg->Ver[i]);

        // length
        send_buf.push_back((uint8_t)msg->desc_length);
        send_buf.push_back((uint8_t)(msg->desc_length>>8));
        int cnt = 0;
        for(;cnt < msg->desc_length;cnt++)
        {
            send_buf.push_back(msg->desc[cnt]);
            if(cnt > 1024)
                break;
        }
        send_to_ui_queue.enqueue(send_buf);
}



