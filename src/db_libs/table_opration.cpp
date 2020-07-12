#include <sqlite3.h>
#include <string.h>
#include <stdio.h>

#include "table_opration.h"


//!
//! \brief debug db_name
//!
const char * db_name = "/userdata/debug.db";

//! local db name define
//!
const char * role_action_db = "./role_action_.db";

//! json interface define
const char * j_add_list = "add_list";
const char * j_del_list = "delete_list";
const char * j_edt_list = "edit_list";
const char * j_data  = "data";

//!
const char * data_list = "insert_list";
const char * event_list = "add_list";
const char * del_list = "data";

//! cloud json format define
const char * key_id = "mid";        //! KEY ID
const char * file_name = "file_name";       //!
const char * file_desc = "desc";       //! 文件描述 多语种数据json集合
const char * f_lib_para = "type";    //const char * f_lib_para = "file_lib_para";  //!int
const char * f_version = "version";

const char * action_place = "place";
const char * role_scenery = "type";      ///> 区分文件库场景, normal， sleep, event of libs
const char * role_type = "category";
const char * f_run_time = "runTime";
const char * run_priority = "priority";

const char * alive_level = "level";
const char * happiness = "happiness";
const char * anger = "anger";
const char * sadness = "sadness";
const char * enjoyment = "enjoyment";

const char * event_num = "Number";          //! event libs field
const char * url_path = "file_path";         //! http:// url / file_name.zip
const char * createt_time = "create_time";  //!  创建时间戳

//! \brief drop table
const char * dropSQL = "DROP TABLE %s";

/**
  * @brief create table, or drop table
  * @param input :json formart context
  * @param output: none
  * @retval 0 is success,other is failure
*/
int table_oprate(char *db_name,char *sql)
{
    sqlite3* conn = NULL;
//! 1. 打开数据库
    int result = sqlite3_open(db_name,&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
    return -1;
    }

//! 2. 准备创建数据表，如果创建失败，需要用sqlite3_finalize释放sqlite3_stmt对象，以防止内存泄露。
    sqlite3_stmt* stmt = NULL;
    int len = strlen(sql);
    printf("%s\n",sql);
    if (sqlite3_prepare_v2(conn,sql,len,&stmt,NULL) != SQLITE_OK) {
        if (stmt)
            sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return -2;
    }

//! 2. 通过sqlite3_step命令执行创建表的语句。对于DDL和DML语句而言，sqlite3_step执行正确的返回值
//只有SQLITE_DONE，对于SELECT查询而言，如果有数据返回SQLITE_ROW，当到达结果集末尾时则返回 SQLITE_DONE。
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return -3;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return 0;
}

//!
//! \brief create normal action table SQL
//!
const char * createNormalSQL =\
"CREATE TABLE NORMAL_ACTION (%s INT PRIMARY KEY NOT NULL,%s TEXT NOT NULL,%s TEXT,%s INT,%s TEXT,\
%s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s TEXT,%s INT);";
//!
//! \brief CreateNormalTable
//! \return
//!
int CreateNormalTable(void)
{
    int ret;
    //! \brief debug create normal table
    char sql[512];
    memset(sql,0,sizeof(sql));
    sprintf(sql,(char*)createNormalSQL,\
            key_id, file_name, file_desc, f_lib_para, f_version,\
            action_place, role_scenery, role_type,f_run_time,run_priority,\
            alive_level, happiness, anger,sadness,enjoyment,\
            event_num,url_path,createt_time);
    ret = table_oprate((char*)db_name, sql); /// create normal table
    return ret;
}
//! \brief create sleep table
const char * createSleepSQL =\
"CREATE TABLE SLEEP_ACTION(%s INT PRIMARY KEY NOT NULL,%s TEXT NOT NULL,%s TEXT,%s INT,%s TEXT,\
%s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s TEXT,%s INT);";
//!
//! \brief CreateSleepTable
//! \return
//!
int CreateSleepTable(void)
{
    int ret;
    //! \brief debug create normal table
    char sql[512];
    memset(sql,0,sizeof(sql));
    sprintf(sql,(char*)createSleepSQL,\
            key_id, file_name, file_desc, f_lib_para, f_version,\
            action_place, role_scenery, role_type,f_run_time,run_priority,\
            alive_level, happiness, anger,sadness,enjoyment,\
            event_num,url_path,createt_time);
    ret = table_oprate((char*)db_name, sql); /// create normal table
    return ret;
}
//! \brief create event table
const char * createEventSQL =\
"CREATE TABLE EVENT_ACTION (%s INT PRIMARY KEY NOT NULL,%s TEXT NOT NULL,%s TEXT,%s INT,%s TEXT,\
%s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s INT,%s INT,%s INT,%s INT,  %s INT,%s TEXT,%s INT);";
//!
//! \brief CreateEventTable
//! \return
//!
int CreateEventTable(void)
{
    int ret;
    //! \brief debug create normal table
    char sql[512];
    memset(sql,0,sizeof(sql));
    sprintf(sql,(char*)createEventSQL,\
            key_id, file_name, file_desc, f_lib_para, f_version,\
            action_place, role_scenery, role_type,f_run_time,run_priority,\
            alive_level, happiness, anger,sadness,enjoyment,\
            event_num,url_path,createt_time);
    ret = table_oprate((char*)db_name, sql); /// create normal table
    return ret;
}


/**
     * @part 1:   判断表是否存在
     * @param tabName 表名
     * @return : 0 is exist, -1 is table name is null,
     *
     * @test   :select count(*) from sqlite_master where type='table' and name='EVENT_ACTION';
     */

int tableIsExist(char * tableName)
{
    int result = false;
    if(tableName == NULL){
        return -1;
    }

    sqlite3 * conn = NULL;
    result = sqlite3_open(db_name,&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
        return -2;
    }

    char sql[512];
    char * selectTableSql = "select count(*) from sqlite_master where type='table' and name = '%s';";
    sqlite3_stmt * stmt = NULL;
    memset(sql,0,sizeof(sql));
    sprintf(sql,selectTableSql,tableName);
    int len = strlen(sql);
    if (sqlite3_prepare_v2(conn,sql,len,&stmt,NULL) != SQLITE_OK ){
        if (stmt)
            sqlite3_finalize(stmt);
        sqlite3_close(conn);
        spdlog::error("OPRATE ERROR: {} ",sql);
        return -3;
    }

    //! \note response result
    if (sqlite3_step(stmt) != SQLITE_DONE){
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return -4;
    }
    return 0;
}

/**
 * @brief table_insert_row
 * @param tb_name
 * @param js
 * @param listType
 * @param row
 * @return
 */
int table_insert_row(char * tb_name, json & js, char *listType, int8_t row)
{

#define SQL_BUFFER_MAX_SIZE 2048

    sqlite3* conn = NULL;
//! 1. 打开数据库
    int result = sqlite3_open(db_name,&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
    return -1;
    }

//! 2. 创建表语句的资源。
    const char* insertSQL = "INSERT INTO %s VALUES(%d,'%s','%s',%d,'%s',  %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, %d,'%s',%d);";

    char sql[SQL_BUFFER_MAX_SIZE];
    sqlite3_stmt* stmt2 = NULL;

//! 3. 构建插入数据的sqlite3_stmt对象。
    int insertCount = 1;

    try{
        //! \note insert row to table
        int keyId = js[j_data][listType][row][key_id];
        std::string fileName = js[j_data][listType][row][file_name];
        std::string fileDesc = js[j_data][listType][row][file_desc];
        int fLibPara = js[j_data][listType][row][f_lib_para];
        std::string fVersion = js[j_data][listType][row][f_version];

        int actionPlace = js[j_data][listType][row][action_place];
        int roleScenery = js[j_data][listType][row][role_scenery];
        int roleType = js[j_data][listType][row][role_type];
        int runTime = js[j_data][listType][row][f_run_time];
        int runPriority = js[j_data][listType][row][run_priority];

        int aliveLevel = js[j_data][listType][row][alive_level];
        int _happiness = js[j_data][listType][row][happiness];
        int _anger = js[j_data][listType][row][anger];
        int _sadness = js[j_data][listType][row][sadness];
        int _enjoyment = js[j_data][listType][row][enjoyment];
        int eventNumber = js[j_data][listType][row][event_num];
        std::string urlPath = js[j_data][listType][row][url_path];
        int createTime = js[j_data][listType][row][createt_time];

        //! insert row values
        memset(sql,0,sizeof(sql));
        sprintf(sql,insertSQL, tb_name,
                    keyId,fileName.data(),fileDesc.data(),fLibPara,fVersion.data(),\
                    actionPlace,roleScenery,roleType,runTime,runPriority,\
                    aliveLevel,_happiness,_anger,_sadness,_enjoyment, eventNumber,urlPath.data(),createTime);
        spdlog::debug(" {} ",sql);

        if (sqlite3_prepare_v2(conn,sql,strlen(sql),&stmt2,NULL) != SQLITE_OK) {
            if (stmt2)
                sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -2;
        }
        if (sqlite3_step(stmt2) != SQLITE_DONE) {
            sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -3;
        }

        //!\part :download resource file



        sqlite3_finalize(stmt2);
        sqlite3_close(conn);
        return 0;
    }catch(std::exception & ex){
        spdlog::error("table_opration.cpp line:{} , ERROR: can not parse json data:{}",__LINE__,ex.what());
        return -4;
    }
}

//!
//! \brief table_update_row
//! \param tb_name
//! \param js
//! \return
//! \note : only one set update
int table_update_row(char * tb_name, json & js,char *listType, int8_t row)
{
    sqlite3* conn = NULL;
//! 1. 打开数据库
    int result = sqlite3_open(db_name,&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
    return -1;
    }

//! 2. 创建表语句的资源。
    const char* updateSQL = "UPDATE %s SET %s='%d',%s='%s',%s='%s',%s='%d',%s='%s',\
    %s='%d',%s='%d',%s='%d',%s='%d',%s='%d',  %s='%d',%s='%d',%s='%d',%s='%d',%s='%d',  %s='%d',%s='%s',%s='%d' WHERE %s=%d;";

    char sql[1024];
    sqlite3_stmt* stmt2 = NULL;

//! 3. 构建插入数据的sqlite3_stmt对象。

    try{
        //! \note insert row to table
        int keyId = js[j_data][listType][row][key_id];
        std::string fileName = js[j_data][listType][row][file_name];
        std::string fileDesc = js[j_data][listType][row][file_desc];
        int fLibPara = js[j_data][listType][row][f_lib_para];
        std::string fVersion = js[j_data][listType][row][f_version];

        int actionPlace = js[j_data][listType][row][action_place];
        int roleScenery = js[j_data][listType][row][role_scenery];
        int roleType = js[j_data][listType][row][role_type];
        int runTime = js[j_data][listType][row][f_run_time];
        int runPriority = js[j_data][listType][row][run_priority];

        int aliveLevel = js[j_data][listType][row][alive_level];
        int _happiness = js[j_data][listType][row][happiness];
        int _anger = js[j_data][listType][row][anger];
        int _sadness = js[j_data][listType][row][sadness];
        int _enjoyment = js[j_data][listType][row][enjoyment];

        int eventNumber = js[j_data][listType][row][event_num];
        std::string urlPath = js[j_data][listType][row][url_path];
        int createTime = js[j_data][listType][row][createt_time];

        //! update row values
        memset(sql,0,sizeof(sql));
        sprintf(sql,updateSQL, tb_name,\
            key_id,keyId,file_name,fileName.data(),file_desc,fileDesc.data(),f_lib_para,fLibPara,f_version,fVersion.data(),\
            action_place,actionPlace, role_scenery,roleScenery, role_type,roleType, f_run_time,runTime,run_priority,runPriority,\
            alive_level,aliveLevel,happiness,_happiness, anger,_anger, sadness,_sadness, enjoyment,_enjoyment,\
            event_num,eventNumber,url_path,urlPath.data(),createt_time,createTime,  key_id,keyId);
        spdlog::debug(" {} ",sql);

        if (sqlite3_prepare_v2(conn,sql,strlen(sql),&stmt2,NULL) != SQLITE_OK) {
            if (stmt2)
                sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -2;
        }
        if (sqlite3_step(stmt2) != SQLITE_DONE) {
            sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -3;
        }


        sqlite3_finalize(stmt2);
        sqlite3_close(conn);
        return 0;
    }catch(std::exception & ex){
        spdlog::error("insert.cpp line:{} , ERROR: can not parse json data:{}",__LINE__,ex.what());
        return -1;
    }

}
//!
//! \brief table_delete_row
//! \param tb_name
//! \param js : format => key : values
//! \return 0 is delete row element success
//!
int table_delete_row(char * tb_name, json & js, char *listType, int8_t row)
{
    sqlite3* conn = NULL;
//! 1. 打开数据库
    int result = sqlite3_open(db_name,&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
    return -1;
    }

//! 2. 创建表语句的资源。
    const char* deleteSQL = "DELETE FROM %s WHERE %s = %d;";

    char sql[512];
    sqlite3_stmt* stmt2 = NULL;

//! 3. 构建插入数据的sqlite3_stmt对象。
    try{
        int keyId = js[j_data][listType][row][key_id];

        //! delete row sql
        memset(sql,0,sizeof(sql));
        sprintf(sql,deleteSQL, tb_name,key_id,keyId);
        spdlog::debug(" {} ",sql);

        if (sqlite3_prepare_v2(conn,sql,strlen(sql),&stmt2,NULL) != SQLITE_OK) {
            if (stmt2)
                sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -2;
        }

        if (sqlite3_step(stmt2) != SQLITE_DONE) {
            sqlite3_finalize(stmt2);
            sqlite3_close(conn);
            return -3;
        }

        sqlite3_finalize(stmt2);
        sqlite3_close(conn);
        return 0;
    }catch(std::exception & ex){
        spdlog::error("insert.cpp line:{} , ERROR: can not parse json data:{}",__LINE__,ex.what());
        return -1;
    }
}


