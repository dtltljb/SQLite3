#ifndef SELECT_H
#define SELECT_H

#include <sqlite3.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <map>
#include <iostream>

//! thirty libs
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "nlohmann/json.hpp"
#include "blockingconcurrentqueue.h"

using namespace std;
using namespace moodycamel;
using json = nlohmann::json;



/**  dbResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据
 *   dbResult 的字段值是连续的，从第0索引到第 nColumn - 1索引都是字段名称，
 * 从第 nColumn 索引开始，后面都是字段值，
 * 它把一个二维的表（传统的行列表示法）用一个扁平的形式来表示
 */
typedef struct{
    char * errmsg = NULL;
    char **dbResult; //是 char ** 类型，两个*号
    int nRow, nColumn;
}select_result_set;

int mid_get_name_int_time(char * tb_name,int mid, std::map<std::string,int> & map);

/**
 * @brief row_get_name_createtime
 * @param tb_name
 * @param row   : by row
 * @param map   : std::pair<std::string,std::string>( fileName, createTime )
 * @return  : 0 is sucess , map.size != 0 result output
 */
int row_get_name_createtime(char * tb_name,int row, std::map<std::string,std::string> & map);

/**
 * @brief by_row_query_name_createtime
 * @param mid
 * @param map
 * @return
 */
int by_row_query_name_createtime(int mid, std::map<std::string,std::string> & map);

/**
 * @brief by_mid_query_name_int_time
 * @param mid
 * @param map
 * @return  : 0 success, 1 empty
 */
int by_mid_query_name_int_time(int mid, std::map<std::string,int> & map);

//!
//! \brief query_table_mid_createtime
//! \param tb_name
//! \param map : mid & create_time
//! \return    : 0 is success, 1 empty table,< 0 failure
//!
int query_table_mid_createtime(char * tb_name, std::map<int, int> &map);

/**
 * @brief query_catalog_key_pair
 * @param catalog_file : device catalog file
 * @param catalog      : std::map<int,int> ( mid & create_time )
 * @return             : 0 success, 1 catalog_file empty, < 0 is error
 */
int query_catalog_mid_createtime(const char * catalog_file, std::map<int, int> &catalog);

/**
 * @brief query_catalog_mid_name_time
 * @param catalog_file
 * @param mid
 * @param catalog
 * @return
 */
int query_catalog_mid_name_time(const char * catalog_file, int mid,
                                std::map<string, int> &catalog);
//!
//! \brief query_table_key_value
//! \param tb_name
//! \param key  :search key words
//! \param val  :result set content
//! \return 0 is search success , < 0 is error,1 table is empty,2 table no key words
//!
int query_table_key_value(char * tb_name,char * key, std::vector<std::string> & val);

//!
//! \brief query_total_table_mid
//! \param key
//! \param list :list empty,no find mach key words
//! \return :0 is success
//!
int query_total_table_mid(char *key, std::vector<std::string> &list);

/**
 * @brief query_insert_to_device_file
 * @param dev_file : device catalog file
 * @param * extend : device file extend name
 * @param   map    : mid & file name
 * @return
 */
int query_insert_to_device_file(const char * catalog_file, const char * extend, std::map<int, string> &map);


/**
 * @brief query_update_to_device_file
 * @param catalog_file
 * @param
 * @return
 */
int query_update_to_device_file(const char * catalog_file, const char *extend, std::map<int, string> & map);

/**
 * @brief query_delete_to_device_file
 * @param catalog_file
 * @param
 * @return
 */
int query_delete_to_device_file(const char * catalog_file, const char *extend, std::map<int, string> &map);

/**
 * @brief catalog_file_insert_row
 * @param catalog_file
 * @param mid
 * @return
 *  查数据库获取文件，填充新增 row 内容
 */
int catalog_file_insert_row(const char* catalog_file, int mid);

/**
 * @brief catalog_file_update_row
 * @param catalog_file
 * @param mid
 * @return
 * 查数据库获取文件，更新文件中 mid 内容
 */

int catalog_file_update_row(const char* catalog_file, int mid);

/**
 * @brief catalog_file_delete_row
 * @param catalog_file
 * @param mid
 * @return
 */
int catalog_file_delete_row(const char* catalog_file, int mid);

/**
 * @brief create_init_catalog_file
 * @param catalog_file
 * @return
 */
int create_init_catalog_file(const char* catalog_file);
/**
 * @brief get_catalog_file_library_version
 * @param catalog_file
 * @return
 */
std::string get_catalog_file_library_version(const char* catalog_file);

/**
 * @brief normalize_string_sort_result
 * @param f_name
 * @return
 */
int normalize_string_sort_result(const char * f_name);

std::string merger_file_path_extend(const char * f_name, const char * extend, int mid);

#endif // SELECT_H
