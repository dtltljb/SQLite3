#ifndef UPDATE_ROLE_LIBRARY_H
#define UPDATE_ROLE_LIBRARY_H

#include <iostream>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <thread>
#include <fcntl.h>
#include <mqueue.h>
//! thirty Libs
#include <sqlite3.h>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "nlohmann/json.hpp"
#include "blockingconcurrentqueue.h"

using json = nlohmann::json;


class update_role_library
{
public:
    update_role_library(void);
    int check_library_progress(char *monitorFile);
    int synchornize_library(char * jsonFile, char *progressFile);
    void update_role_library_entry(void);

private:
    int tableIsExist(char * tableName);

};

#endif // UPDATE_ROE_LIBRARY_H
