
//! user header
#include "select.h"
#include "table_opration.h"
#include "update_version.hpp"

#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map/fifo_map.hpp"

// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//template<class K, class V, class dummy_compare, class A>
//using my_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
//using my_json = basic_json<my_workaround_fifo_map>;

//!
//! \brief select_table_sql
//! \param db_name
//! \param sql
//! \return
//!
int select_table_sql( char *db_name, char *sql,select_result_set * result_set )
{
    sqlite3* conn = NULL;
    //1. 打开数据库
    int result = sqlite3_open(db_name, &conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
        return -1;
    }
    //! 6. 根据select语句的对象，获取结果集中的字段数量。
//        char * errmsg = NULL;
//        char **dbResult; //是 char ** 类型，两个*号
//        int nRow, nColumn;

        int i , j;

        result = sqlite3_get_table( conn, sql, &result_set->dbResult, &result_set->nRow, &result_set->nColumn, &result_set->errmsg );
        if ( result != SQLITE_OK ){
            // sqlite3_free_table( dbResult );
             sqlite3_close( conn );
             spdlog::debug(" >>> ' {} ' run failure... ",sql);
             return -2;
        }
        //spdlog::debug(" {} , row:{} ,column:{} ",sql, result_set->nRow, result_set->nColumn);
        //printf(" %s , row:%d ,column:%d \n",sql, result_set->nRow, result_set->nColumn);
        /**  dbResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据
         *   dbResult 的字段值是连续的，从第0索引到第 nColumn - 1索引都是字段名称，
         * 从第 nColumn 索引开始，后面都是字段值，
         * 它把一个二维的表（传统的行列表示法）用一个扁平的形式来表示
         */
//        int index;
//        index = result_set->nColumn;
//        for(  i = 0; i < result_set->nRow ; i++ )
//        {
//            //spdlog::debug( "第 {} 条记录", i+1 );
//            printf("第 %d 条记录 \n", i+1 );
//            for( j = 0 ; j < result_set->nColumn; j++ )
//            {
//                //spdlog::debug( "字段名: {}  字段值: {} ",  result_set->dbResult[j], result_set->dbResult[index] );
//                printf( "字段名: %s  字段值: %s \n",  result_set->dbResult[j], result_set->dbResult[index] );
//                ++index;
//            }
//            spdlog::debug("-------------------" );
//        }
        //! sqlite3_free_table( dbResult ); 调用程序负责释放
        sqlite3_close(conn);
        return 0;
}

/**
 @note     查询 tableName 表 TYPE = 1 按照ModifiedTime 降序 限制20条，跳过第一条数据
 @brief    select * from tableName where TYPE = 1 order by ModifiedTime desc Limit 20 Offset 1
           select * from NORMAL_ACTION limit 1;
           select * from NORMAL_ACTION where mid > last_mid limit 1;
*/

/**
 * @brief row_get_name_createtime
 * @param tb_name
 * @param row   : by row
 * @param map   : std::pair<std::string,std::string>( fileName, createTime )
 * @return  : 0 is sucess , map.size != 0 result output
 */
int row_get_name_createtime(char * tb_name,int row, std::map<std::string,std::string> & map)
{

    const char * lastSql = "select * from %s where mid == %d;";
    char sql[512];

    select_result_set res;
    std::string fileName;
    std::string createTime;
    int index;
    int i,j;
    int ret;

    memset(sql,0,sizeof(sql));
    sprintf(sql,(char*)lastSql,tb_name,row);
    ret = select_table_sql( (char*)db_name,(char*)sql,&res );
    index = res.nColumn;
    if (res.nRow == 0){
        spdlog::info("sql: {},-- query is empty..",sql);
        return -1;          //! query row empty
    }

    for ( i=0;i < res.nRow; i++ ){
        for ( j=0; j < res.nColumn; j++){
            switch (j)
            {
            case 1:             //!file_name
                fileName = res.dbResult[index];
                break;
            case 17:            //!createt_time
                createTime = res.dbResult[index];
                ret = 2;
                break;
            default:
                break;
            }
            index++;
        }
    }
    //! query result set insert map
    if(ret ==2){
        map.insert(std::pair<std::string,std::string>( fileName, createTime) );
        return 0;
    }else{
        spdlog::info("sql: {} ",sql);
        return 1;  ///> db content error
    }

}
/**
 * @brief by_row_query_name_createtime
 * @param mid   : mid
 * @param map   : std::pair<std::string,std::string>( fileName, createTime )
 * @return
 */
int by_mid_query_name_createtime(int mid, std::map<std::string,std::string> & map)
{

    int ret;
    ret = row_get_name_createtime((char*)normal_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    ret = row_get_name_createtime((char*)event_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    ret = row_get_name_createtime((char*)sleep_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    return 1;
}


int mid_get_name_int_time(char * tb_name,int mid, std::map<std::string,int> & map)
{
    const char * lastSql = "select * from %s where mid == %d;";
    char sql[512];

    select_result_set res;
    std::string fileName;
    std::string createTime;
    int int_time;
    int index;
    int i,j;
    int ret;
    memset(sql,0,sizeof(sql));
    sprintf(sql,(char*)lastSql,tb_name,mid);
    ret = select_table_sql( (char*)db_name,(char*)sql,&res );
    index = res.nColumn;

    if (res.nRow == 0){
        spdlog::info("sql: {} query is empty",sql);
        return -1;          //! query row empty
    }

    for ( i=0;i < res.nRow; i++ ){
        for ( j=0; j < res.nColumn; j++){
            switch (j)
            {
            case 1:             //!file_name
                fileName = res.dbResult[index];
                break;
            case 17:            //!createt_time
                createTime = res.dbResult[index];
                int_time = std::atoi(createTime.c_str());
                ret =2;
                break;
            default:
                break;
            }
            index++;
        }
    }
    //! query result set insert map
    if(ret ==2){
       map.insert(std::pair<std::string,int>( fileName, int_time) );
        return 0;
    }else{
        spdlog::info("sql: {} ",sql);
        return 1;  ///> db content error
    }

}
/**
 * @brief by_mid_query_name_int_time
 * @param mid
 * @param map
 * @return  : 0 success, 1 empty
 */
int by_mid_query_name_int_time(int mid, std::map<std::string,int> & map)
{

    int ret;
    ret = mid_get_name_int_time((char*)normal_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    ret = mid_get_name_int_time((char*)event_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    ret = mid_get_name_int_time((char*)sleep_table,mid,map);
    if( map.size() == 1 ){
        return 0;
    }
    return 1;
}

/**
 * @brief query_table_mid_createtime
 * @param tb_name
 * @param map   :  mid && create_time
 * @return      : 0 is success, 1 empty table,< 0 failure
 */
int query_table_mid_createtime(char * tb_name,std::map<int,int> & map)
{
    //const char * sqlTop = "select * from %s where mid > 0 limit 1;";
    const char * lastSql = "select * from %s where mid > %s limit 1;";
    char sql[512];
    //char msg[256];

    select_result_set res;
    std::string lastMid = "0";
    std::string createTime;
    int index; int i,j;
    int ret;    
    int count = 1;
    int flag = 1;
    while(flag)
    {
        memset(sql,0,sizeof(sql));
        sprintf(sql,(char*)lastSql,tb_name,(char*)lastMid.data());
        ret = select_table_sql( (char*)db_name,(char*)sql,&res );
        index = res.nColumn;
        if (res.nRow == 0){
            flag = 0;       //! query row nothing
            return 0;
        }
        for ( i=0;i < res.nRow; i++ ){
            for ( j=0; j < res.nColumn; j++){
                switch (j)
                {
                case 0:             //!mid
                    lastMid = res.dbResult[index];
                    break;
                case 17:            //!create
                    createTime = res.dbResult[index];
                    break;
                default:
                    break;
                }
                index++;
            }
        }
        //! query result set insert map
        map.insert(std::pair<int,int>( std::atoi(lastMid.c_str()), std::atoi(createTime.c_str()) ));

        count += 1;
        if (count > 1000 ){
            spdlog::error("数据表 {} 内容超出 {} 条记录；请厂家服务人员!!!", tb_name, count);
            break;
        }
    }
}

/**
 * @brief query_table_mid_file_name
 * @param tb_name
 * @param map       : mid & fileName
 * @return
 */
int query_table_mid_file_name(char * tb_name,std::map<int,std::string> & map)
{
    //const char * sqlTop = "select * from %s where mid > 0 limit 1;";
    const char * lastSql = "select * from %s where mid > %s limit 1;";
    char sql[512];
    //char msg[256];

    select_result_set res;
    std::string mid = "0";
    std::string file_name;
    int index; int i,j;
    int ret;
    int count = 1;
    int flag = 1;
    int type =LIB_TYPE_FILE;

    while(flag)
    {
        memset(sql,0,sizeof(sql));
        sprintf(sql,(char*)lastSql,tb_name,(char*)mid.data());
        ret = select_table_sql( (char*)db_name,(char*)sql,&res );
        index = res.nColumn;
        if (res.nRow == 0){
            flag = 0;       //! query row nothing
            sqlite3_free_table( res.dbResult );
            return 0;
        }

        for ( i=0;i < res.nRow; i++ ){
            for ( j=0; j < res.nColumn; j++){
                switch (j)
                {
                case 0:             //!mid
                    mid = res.dbResult[index];
                    break;
                case 1:            //!file name
                    file_name = res.dbResult[index];
                    break;
                case 3://file_lib_para
                    type = std::atoi(res.dbResult[index]);
                    break;
                default:
                    break;
                }
                index++;
            }
        }
        //! query result set insert map
        if( static_cast<lib_type_para_e>(type) == LIB_TYPE_FILE){
            map.insert(std::pair<int,std::string>(std::atoi(mid.c_str()), file_name) );
        }

        count += 1;
        if (count > 1000 ){
            spdlog::error("数据表 {} 内容超出 {} 条记录；请厂家服务人员!!!", tb_name, count);
            break;
        }
    }
}

/**
 * @brief query_catalog_key_pair
 * @param catalog_file : device catalog file
 * @param catalog      : std::map<int,int> ( mid & create_time )
 * @return             : 0 success, 1 catalog_file empty, < 0 is error
 */
int query_catalog_mid_createtime(const char * catalog_file, std::map<int,int> &catalog)
{

    /**
     * @note device store catalog file content
    */
    json dev_catalog ={
        {"version","0.0.0.1"},
        {"sn","0"},
        {"list",{
             {{"mid",0},{"time",0},{"name",""}},
                 }
        }
    };

    std::ifstream in_file(catalog_file);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",catalog_file, size);
        return -2;
    }

    int count = 0;
    int f_mid, c_time;
    //int r = *row;
    try{
        json js = json::parse(context);
        count = js["list"].size();
        if (count == 0 ){
            spdlog::info(" {} mid & time list[] is 0 count,abort exit..",catalog_file);
            return 1;
        }
        int i = 0;
        for(;i < count; i++){
            f_mid = js["list"][i]["mid"];
            c_time = js["list"][i]["time"];
            catalog.insert(std::pair<int,int>(f_mid,c_time));
        }
        return 0;

    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -3;
    }

}
/**
 * @brief query_catalog_mid_name_time
 * @param catalog_file
 * @param mid
 * @param catalog :name & time
 * @return :0 success , 1 empty , < 0 failure
 */
int query_catalog_mid_name_time(const char * catalog_file, int mid,
                                std::map<std::string,int> &catalog)
{

    /**
     * @note device store catalog file content
    */
    json dev_catalog ={
        {"version","0.0.0.1"},
        {"sn","0"},
        {"list",{
             {{"mid",0},{"time",0},{"name",""}},
                 }
        }
    };

    std::ifstream in_file(catalog_file);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",catalog_file, size);
        return -2;
    }

    int count = 0;
    int f_mid,c_time;
    std::string f_name;
    //int r = *row;
    try{
        json js = json::parse(context);
        count = js["list"].size();
        if (count == 0 ){
            spdlog::info(" {} mid & time list[] is 0 count,abort exit..",catalog_file);
            return 1;
        }
        int i = 0;
        for(;i < count; i++){
            f_mid = js["list"][i]["mid"];
            if(mid == f_mid){
                c_time = js["list"][i]["time"];
                f_name = js["list"][i]["name"];
                catalog.insert(std::pair<std::string,int>(f_name,c_time));
                return 0;
            }
        }
        return 1;
    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -3;
    }

}

//!
//! \brief query_table_key_value
//! \param tb_name
//! \param key  :search key words
//! \param val  :result set content
//! \return 0 is search success , < 0 is error,1 table is empty,2 table no key words
//!
int query_table_key_value(char * tb_name,char *key, std::vector<std::string> & val)
{
    //const char * sqlTop = "select * from %s where mid > 0 limit 1;";
    const char * lastSql = "select * from %s where mid > %s limit 1;";
    char sql[512];
    //char msg[256];

    int r = 0;
    int index; int i,j;
    int ret;

    //! \note get table top 1 row content
    //int lastMid = 0;
    //int createTime = 0;
    select_result_set res;
    std::string k1,k2 =key;
    std::string lastMid ="0";

    int count = 1;
    int flag = 1;
    while(flag)
    {
        memset(sql,0,sizeof(sql));
        sprintf(sql,(char*)lastSql,tb_name,(char*)lastMid.data());
        ret = select_table_sql( (char*)db_name,(char*)sql,&res );
        index = res.nColumn;
        if (res.nRow == 0){
            flag = 0;//! query row nothing
            return 0;
        }
        for ( i=0;i < res.nRow; i++ ){
            for ( j=0; j < res.nColumn; j++){
                k1 = res.dbResult[j];
                if (k1 == k2 ){
                    val.push_back(res.dbResult[index]);
                    //val.append(",");
                    lastMid = res.dbResult[index];
                    r += 1;    //! search content count values
                }
                index++;
            }
        }
        count += 1;
        if (count > 1000 ){
            spdlog::error("数据表 {} 内容超出 {} 条记录；请厂家服务人员!!!", tb_name, count);
            break;
        }
    }
    return -1;
}

//!
//! \brief query_total_table_mid
//! \param key
//! \param list :list empty,no find mach key words
//! \return :0 is success
//!
int query_total_table_mid(char *key, std::vector<std::string> &list)
{
    int ret;
    //std::vector<std::string> mid_list;
    ret = query_table_key_value((char*)normal_table,key,list);
    if(ret < 0 )
    {
        spdlog::error("query key {} table {} happen failure event...",key,(char*)normal_table);
    }

    ret = query_table_key_value((char*)event_table,key,list);
    if (ret < 0 ){
        spdlog::error("query key {} table {} happen failure event...",key,(char*)event_table);
    }

    ret = query_table_key_value((char*)sleep_table,key,list);
    if (ret < 0 ){
        spdlog::error("query key {} table {} happen failure event...",key,(char*)sleep_table);
    }

    return 0;
}


/**
 * @brief query_insert_to_device_file
 * @param dev_file : device catalog file
 * @param * extend : device file extend name
 * @param   map    : mid & file name
 * @return
 */
int query_insert_to_device_file(const char * catalog_file, const char *extend, std::map<int, string> &map)
{
    int mid;

    int ret;

    std::map<int,std::string>::iterator iter;
    std::map<int,std::string> db_list;
    ret = query_table_mid_file_name((char*)normal_table, db_list);
//    ret = query_table_mid_file_name((char*)event_table, db_list);
//    ret = query_table_mid_file_name((char*)sleep_table, db_list);

    std::map<int,int> catalog;
    std::map<int,int>::iterator it;
    query_catalog_mid_createtime(catalog_file, catalog);

    iter = db_list.begin();
    for ( ;iter != db_list.end();iter++){
        mid = iter->first;

        it = catalog.find(mid);
        if(it == catalog.end()){
            std::string path = LIBRARY_BASE_PATH;
            char tmp[16];
            memset(tmp,0,sizeof(tmp));
            std::sprintf(tmp,"/%d/",mid);
            path.append(tmp);
            path += iter->second;
            int pos = path.find_first_of('.');
            std::string s = path.substr(pos+1);
            path.replace(pos+1,strlen(extend),extend);

            std::ifstream in_file(path);
            if(in_file.is_open())
                map.insert(std::pair<int,std::string>(mid,iter->second) );
            in_file.close();
            continue;
        }
    }
    return 0;
}

/**
 * @brief query_update_to_device_file
 * @param catalog_file
 * @param extend
 * @return
 */
int query_update_to_device_file(const char * catalog_file,const char * extend, std::map<int,std::string> & map)
{
    int mid,c_time;
    int ret;
    std::map<int,int>::iterator iter;
    std::map<int,int> db_list;
    ret = query_table_mid_createtime((char*)normal_table, db_list);
    ret = query_table_mid_createtime((char*)event_table, db_list);
    ret = query_table_mid_createtime((char*)sleep_table, db_list);

    std::map<int,int> catalog;
    std::map<int,int>::iterator it;
    query_catalog_mid_createtime(catalog_file, catalog);
    if(catalog.size() == 0){///> 无更新内容
        return 1;
    }

    std::map<std::string,std::string> name_time;
    std::map<std::string,std::string>::iterator nt;

    it = catalog.begin();
    for(;it != catalog.end();it++){

        mid = it->first;
        c_time = it->second;

        iter = db_list.find(mid);
        if (iter != db_list.end()){
            if(c_time != iter->second ){
                name_time.clear();
                by_mid_query_name_createtime(mid,name_time);
                nt = name_time.begin();

                std::string path = LIBRARY_BASE_PATH;
                char tmp[16];
                memset(tmp,0,sizeof(tmp));
                std::sprintf(tmp,"/%d/",mid);
                path.append(tmp);
                path += nt->first;
                int pos = path.find_first_of('.');
                std::string s = path.substr(pos+1);
                path.replace(pos+1,strlen(extend),extend);

                std::ifstream in_file(path);
                if(in_file.is_open()){
                    map.insert(std::pair<int,std::string>(mid,nt->first) );
                }
                in_file.close();
            }
        }else{
            /** @todo nothing */
        }
    }
    return 0;
}

/**
 * @brief query_delete_to_device_file
 * @param catalog_file
 * @param row
 * @return
 */
int query_delete_to_device_file(const char * catalog_file,const char * extend, std::map<int,std::string> & map)
{

    int mid;
    int ret;
    std::map<int,int>::iterator iter;
    std::map<int,int> db_list;
    ret = query_table_mid_createtime((char*)normal_table, db_list);
    ret = query_table_mid_createtime((char*)event_table, db_list);
    ret = query_table_mid_createtime((char*)sleep_table, db_list);

    std::map<int,int> catalog;
    std::map<int,int>::iterator it;
    query_catalog_mid_createtime(catalog_file, catalog);

    std::map<std::string,int> name_time;
    std::map<std::string,int>::iterator nt;
    it = catalog.begin();
    for(;it != catalog.end();it++){

        mid = it->first;

        iter = db_list.find(mid);
        if (iter == db_list.end()){
            name_time.clear();
            query_catalog_mid_name_time(catalog_file, mid, name_time);
            nt = name_time.begin();

            std::string path = LIBRARY_BASE_PATH;
            char tmp[16];
            memset(tmp,0,sizeof(tmp));
            std::sprintf(tmp,"/%d/",mid);
            path.append(tmp);
            path += nt->first;
            int pos = path.find_first_of('.');
            std::string s = path.substr(pos+1);
            path.replace(pos+1,strlen(extend),extend);
            map.insert(std::pair<int,std::string>(mid,nt->first) );

//            std::ifstream in_file(path);
//            if(in_file.is_open()){
//                map.insert(std::pair<int,std::string>(mid,nt->first) );
//            }
//            in_file.close();
        }else{
            /** @todo nothing */
        }
    }

    return 0;
}

/**
 * @brief catalog_file_insert_row
 * @param catalog_file
 * @param mid
 * @return
 *  查数据库获取文件，填充新增 row 内容
 */
int catalog_file_insert_row(const char* catalog_file, int mid)
{

    /**
     * @note device store catalog file content
    */
    json dev_catalog ={
        {"version","0.0.0.1"},
        {"sn","0"},
        {"list",{ {}, } }
    };

    std::ifstream in_file(catalog_file);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",catalog_file, size);
        return -2;
    }

    int index = 0;
    std::map<std::__cxx11::string,int> list;
    std::map<std::__cxx11::string,int>::iterator iter;
    int ret = by_mid_query_name_int_time(mid,list);
    if(ret ==1){
        spdlog::error("database table no search row content..");
        return 1;
    }
    iter = list.begin();

    try{
        json js = json::parse(context);
        index = js["list"].size();
        js["list"][index]["mid"] = mid;
        js["list"][index]["name"] = iter->first;
        js["list"][index]["time"] = iter->second;

        context.clear();
        context = js.dump();
        std::ofstream o_file(catalog_file);
        if(! o_file.is_open()){
            spdlog::debug(" write open {} faile... ",catalog_file);
            return -3;
        }
        o_file << context;
        o_file.close();
        return 0;
    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -4;
    }
}

/**
 * @brief catalog_file_update_row
 * @param catalog_file
 * @param mid
 * @return
 * 查数据库获取文件，更新文件中 mid 内容
 */

int catalog_file_update_row(const char* catalog_file, int mid)
{
    std::ifstream in_file(catalog_file);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",catalog_file, size);
        return -2;
    }

    int index = 0; int f_mid; int i;
    std::map<std::__cxx11::string,int> list;
    std::map<std::__cxx11::string,int>::iterator iter;
    int ret = by_mid_query_name_int_time(mid,list);
    if(ret ==1){
        spdlog::error("database table no search row content..");
        return 1;
    }
    iter = list.begin();


    try{
        json js = json::parse(context);
        index = js["list"].size();
        for( i=0;i<index;i++){
            f_mid = js["list"][i]["mid"];
            if(f_mid == mid){
                break;
            }
        }
        if(i < index){
            js["list"][i]["mid"] = mid;
            js["list"][i]["name"] = iter->first;
            js["list"][i]["time"] = iter->second;
        }else{
            spdlog::info("catalog file no search mid row...");
            return 2;
        }

        context.clear();
        context = js.dump();
        std::ofstream o_file(catalog_file);
        if(! o_file.is_open()){
            spdlog::debug(" write open {} faile... ",catalog_file);
            return -3;
        }
        o_file << context;
        o_file.close();
        return 0;
    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -4;
    }
}

/**
 * @brief catalog_file_delete_row
 * @param catalog_file
 * @param mid
 * @return
 */
int catalog_file_delete_row(const char* catalog_file, int mid)
{
    std::ifstream in_file(catalog_file);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    if (size < 10 ){
        spdlog::error(" {} file content too few {} ,exit..",catalog_file, size);
        return -2;
    }

    int index = 0; int f_mid; int i;
    std::map<std::__cxx11::string,int> list;
    std::map<std::__cxx11::string,int>::iterator iter;
    int ret = by_mid_query_name_int_time(mid,list);
    if(ret ==1){
        spdlog::error("database table no search row content..");
        return 1;
    }
    iter = list.begin();
    std::string version;
    try{
        json js = json::parse(context);
        version = js["version"];
        index = js["list"].size();
        for( i=0;i<index;i++){
            f_mid = js["list"][i]["mid"];
            if(f_mid == mid){
                break;
            }
        }
        if(i < index){
            js["list"].erase(i);        ///> index erase mothed
        }else{
            spdlog::info("catalog file no search mid row...");
            return 2;
        }

        js["version"] = version;
        context.clear();
        context = js.dump();
        std::ofstream o_file(catalog_file);
        if(! o_file.is_open()){
            spdlog::debug(" write open {} faile... ",catalog_file);
            return -3;
        }
        o_file << context;
        o_file.close();
        return 0;
    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -4;
    }
}



/**
 * @brief create_init_catalog_file
 * @param catalog_file
 * @return
 */
int create_init_catalog_file(const char* catalog_file)
{
    /**
     * @note device store catalog file content
    */
    json dev_catalog ={
        {"version","0.0.0.1"},
        {"sn","keyi-2020-03-15"},
        {"list",{ } }
    };

    std::string context;
    try{

        context = dev_catalog.dump();
        std::ofstream o_file(catalog_file);
        if(! o_file.is_open()){
            spdlog::debug(" write open {} faile... ",catalog_file);
            return -1;
        }
        o_file << context;
        o_file.close();
        return 0;
    }catch(std::exception &e){
        spdlog::error("version_infro.cpp,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return -1;
    }
}


/**
 * @brief get_catalog_file_library_version
 * @param catalog_file
 * @return
 */
std::string get_catalog_file_library_version(const char* catalog_file)
{
    std::ifstream in_file(catalog_file);
    std::string context;
    std::string version;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",catalog_file);
        in_file.close();
        return version;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    try{
        json js = json::parse(context);
        version = js["version"];
        return version;
    }catch(std::exception &e){
        spdlog::error("select.c,L:{}, ERROR: CAN NOT PARSE JSON DATA: {}", __LINE__, e.what());
        return version;
    }
}

std::string merger_file_path_extend(const char * f_name, const char * extend, int mid)
{
    std::string path = LIBRARY_BASE_PATH;
    char tmp[16];
    memset(tmp,0,sizeof(tmp));
    std::sprintf(tmp,"/%d/",mid);
    path.append(tmp);
    path += f_name;
    int pos = path.find_first_of('.');
    std::string s = path.substr(pos+1);
    path.replace(pos+1,strlen(extend),extend);
    return path;
}

/**
 * @brief normalize_string_sort_result
 * @param f_name
 * @return
 */
int normalize_string_sort_result(const char * f_name)
{
    std::ifstream in_file(f_name);
    std::string context;
    if(!in_file.is_open()){
        spdlog::debug(" open {} faile ",f_name);
        in_file.close();
        return -1;
    }

    in_file.seekg(0,std::ios::end);
    int size = in_file.tellg();
    in_file.seekg(0,std::ios::beg);
    context.assign(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());
    in_file.close();
    std::string str = "\"version\"";
    std::string::size_type pos = context.find(str);
    if (pos == std::string::npos)
    {
        cout << "Not find" << endl;
        return -1;
    }
    if(pos < 2){
        return 1;
    }

    char buf[32];
    std::string::size_type pos1 = context.find(",",pos,1);
    if (pos1 != std::string::npos)
    {
        memset(buf,0,sizeof(buf));
        context.copy(buf,pos1-pos+1,pos);
        context.erase(pos,pos1-pos+1);
        //context.erase(pos-1,1);
    }else{
        pos1 = context.find("}",pos,1);
        if(pos1 == std::string::npos){
            spdlog::error(" not find smbyle..");
            return -2;
        }
        memset(buf,0,sizeof(buf));
        context.copy(buf,pos1-pos,pos);
        context.erase(pos,pos1-pos);
        context.erase(pos-1,1);
        buf[strlen(buf)] = ',';
    }
    context.insert(1,buf);

    std::ofstream of(f_name);
    if(!of.is_open()){
        spdlog::error(" - {}, open {} failure",__FILE__, f_name);
        return -3;
    }
    of << context;
    of.close();
    return 0;
}

