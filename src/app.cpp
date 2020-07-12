/**
 * @author Li jiabo
 * @brief main file
 * @date 2019/02/26
 * @version 1.0.0
 */

#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <dirent.h>


#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

// thirty libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "concurrentqueue.h"
#include "blockingconcurrentqueue.h"

// user header file
#include "serial/serial.h"

#include "update_version.hpp"

using namespace moodycamel;
extern BlockingConcurrentQueue<std::vector<uint8_t>> send_to_ui_queue;

#define downLoad_url "http://xxxxx.oss-cn-hongkong.aliyuncs.com/abc.zip"

/**
 * @brief main function
 * @return 0 on success
 */

#define SERIAL_RUN_LOG  "/userdata/serial.log"
//#define UBUNTU_DEBUG

int main(void)
{


    std::thread update_firmware_thread(update_firmware_entry); ///> curl通讯, OTA 升级 和 SQLite3 本地库差异化升级  测试程序。
    std::thread db_entry_thread(db_entry);      ///> SQLite3 测试程序入口

    spdlog::info(" Systems build time: {} {}", __DATE__, __TIME__);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    update_firmware_thread.join();
    db_entry_thread.joinable();

	return 0;
}
