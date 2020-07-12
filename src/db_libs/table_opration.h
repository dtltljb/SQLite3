#ifndef INSERT_H
#define INSERT_H

// thirty libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "nlohmann/json.hpp"
#include "blockingconcurrentqueue.h"

using json = nlohmann::json;
using namespace moodycamel;
using namespace std;

//! json interface define
extern const char * j_add_list;
extern const char * j_del_list;
extern const char * j_edt_list;
extern const char * j_data;

//! table values

//! cloud json format define
extern const char * key_id ;//!int
extern const char * file_name;
extern const char * file_desc;  //! 文件描述 多语种数据json集合
extern const char * f_lib_para;//!int
extern const char * f_version;

extern const char * action_place;
extern const char * role_scenery;
extern const char * role_type;
extern const char * f_run_time;
extern const char * run_priority;

extern const char * alive_level;
extern const char * happiness;
extern const char * anger;
extern const char * sadness;
extern const char * enjoyment;

extern const char * event_num;  //! event libs field
extern const char * url_path ;         //! http:// url / file_name.zip
extern const char * createt_time;        //!  创建时间戳

typedef enum{
    LIB_TYPE_FILE = 1,
    LIB_TYPE_ANSWER = 2,
    LIB_TYPE_STORY =3,
    LIB_TYPE_EVNET = 4,
} lib_type_para_e;

//!
//! \brief db_name
//!
extern const char * db_name;
//! create table sql
extern const char * createNormalSQL;
extern const char * createSleepSQL;
extern const char * createEventSQL;
/**
 * @brief db_entry ,数据库测试入口函数
 * @return 无返回值
 */
int db_entry(void);

/**
     * @part 1:   判断表是否存在
     * @param tabName 表名
     * @return : 0 is exist, -1 is table name is null,
     *
     * @test   :select count(*) from sqlite_master where type='table' and name='EVENT_ACTION';
     */

int tableIsExist(char * tableName);

//!
//! \brief table_insert_row
//! \param tb_name :NORMAL_ACTION\SLEEP_ACTION\EVENT_ACTION
//! \param js
//! \param listType :add_list\edit_list\delete_list
//! \param row
//! \return
//!
int table_insert_row(char * tb_name, json & js, char *listType, int8_t row);
//!
//! \brief table_update_row
//! \param tb_name
//! \param js
//! \return
//! \note : only one set update
int table_update_row(char * tb_name, json & js, char *listType, int8_t row);
//!
//! \brief table_delete_row
//! \param tb_name
//! \param js : format => key : values
//! \return 0 is delete row element success
//!
int table_delete_row(char * tb_name, json & js, char *listType, int8_t row);

//!
//! \brief table_insert_event_row
//! \param tb_name
//! \param js
//! \return
//!
int table_insert_event_row(char * tb_name, json & js);

//!
//! \brief CreateNormalTable
//! \return 0 is success,other is failure
//!
int CreateNormalTable(void);

//!
//! \brief CreateSleepTable
//! \return 0 is success,other is failure
//!
int CreateSleepTable(void);
//!
//! \brief CreateEventTable
//! \return 0 is success,other is failure
//!
int CreateEventTable(void);

#endif // INSERT_H
