#ifndef UPDATE_VERSION_HPP
#define UPDATE_VERSION_HPP
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <thread>
#include <fcntl.h>
#include <mqueue.h>

//thirty libs
#include <nlohmann/json.hpp>
#include "blockingconcurrentqueue.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

//user header
#include "version_infor.h"



extern char *get_url_base;
extern char *post_url_base;

extern const char * event_table;
extern const char * sleep_table;
extern const char * normal_table;

extern const char * post_sava_path;
extern const char * monitor_progress;
extern const char * brain_config_file;
extern const char *device_catalog_file;

extern const char * FILE_PATH_NAME;
extern const char * FILE_PATH_NAME_BAK;         ///> bak file
extern const char * FIRMWARE_CONFIG_PATH_FILE;
extern const char * LIBRARY_BASE_PATH;


extern const char * BRAIN_LIB_VER_DEF;
extern const char * LIBRARY_CREATE_TIME;

using namespace moodycamel;     //moodycamel::BlockingConcurrentQueue
using nlohmann::json;
using namespace std;

extern version_infor infor;
extern BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_from_core;
extern BlockingConcurrentQueue<std::vector<uint8_t>> upgrade_queue_to_core;
int update_firmware_entry(void);

extern BlockingConcurrentQueue<std::vector<uint8_t>> rly_send_to_ui_queue;

void rut_send_message_to_ui(mqd_t & mq, struct mq_attr & attr, char * message_buf,
                            uint16_t msg_type, uint16_t index, uint16_t error_code, uint16_t len_of_msg);

void rut_tx_to_ui_thread_entry(void);

/**
 * @brief recursively_download_library
 * @param d_name    :post_in.jscm
 * @param m_file    :monitor_progress.jscm
 * @param c_file    :brain configure.jscm
 * @param url       :url address
 * @return          :0 is download okay, > 0 list is empty, < 0  is error
 */
int recursively_download_library(const char * d_name, const char *m_file, const char *c_file, char *url);

#endif // UPDATE_VERSION_HPP
