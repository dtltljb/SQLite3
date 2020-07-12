#include <sqlite3.h>
#include <string.h>

//! user file
#include "ipc_with_ui.hpp"
#include "select.h"
#include "table_opration.h"
#include "forlder_util.h"
#include "down_load_file.hpp"
#include "update_role_library.h"


BlockingConcurrentQueue<std::vector<uint8_t>> update_role_library_queue;

//!
//! \brief update_role_library::update_role_library
//! \param postJsonFile
//! \return 0 is success,other is error
//!
update_role_library::update_role_library(void)
{

}

/**
     * @part 1:   判断表是否存在
     * @param tabName 表名
     * @return : 0 is exist, -1 is table name is null,
     *
     * @test   :select count(*) from sqlite_master where type='table' and name='EVENT_ACTION';
     */

int update_role_library::tableIsExist(char * tableName)
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
    char * selectTableSql = "select count(*) from sqlite_master where type='table' AND name = '%s' ";
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

//!
//! \brief update_role_library::synchornaze_library
//! \param input: jsonFile post interface response jsolist
//! \param input: progress information File
//! \part 3:检查 进度监控文件中记录下标值和列表名称add_list\del_list\update_list；以此下标和列表类型继续遍历json文件；
//!         实现断点同步库文件的同步能力,本地库同步信息json中 flag 标识为true 表示云服务器还有库信息待同步。
//! \part 4:根据进度信息，add_list\del_list\update_list 执行表操作.
//! \part 5:执行 add 、delete 和update 时，向进度监控文件写当前文件 ID 和 时间戳,同步执行完毕清楚文件内容.
//! \return :0 is synchornize library okay, other is failure
//!
int update_role_library::synchornize_library(char *jsonFile, char *progressFile)
{
    //! \brief drop table
    const char * dropSQL = "DROP TABLE TESTTABLE";
    int ret;
       //! \brief insert table context
    const char * tb_normal = "NORMAL_ACTION";
    const char * tb_sleep = "SLEEP_ACTION";
    json insert_list =
     {
        {"result",200},
        {"msg","操作成功"},
        "data",
            {{"add_list",
            {
                {
                    {"file_key_id",2},
                    {"file_name","abc.zip"},
                    {"file_desc","sleep action"},
                    {"file_lib_para",01},
                    {"file_version","1.0.5.6"},

                    {"action_place",100},
                    {"scenery",2},//2 sleep mode
                    {"role",1},// 1 bac
                    {"run_time",400},// 400 ms
                    {"run_priority",9},

                    {"alive_level",10},
                    {"happiness",3},
                    {"anger",0},
                    {"sadness",0},
                    {"enjoyment",2},

                     {"event_number",10078},
                     {"url_path","http://neuro.coolphper.com/api/firmware/firmwareApi"},
                     {"create_time",10203040506077}
                }
            }
            },
         {"edit_list",
          {
              {"file_key_id",3},
              {"file_name","abc.zip"},
              {"file_desc","sleep action"},
              {"file_lib_para",01},
              {"file_version","1.0.5.6"},

             }
         },
         {"del_list",
          {
              {"file_key_id",4},
              {"file_name","abc.zip"},
              {"file_desc","sleep action"},
              {"file_lib_para",01},
              {"file_version","1.0.5.6"},

          }
         }
        } /** \note object create mothed */
    };

    try{
        //ret = table_insert_row( (char*)tb_normal,insert_list );
        if ( ret != 0 )
            spdlog::debug("table insert error,retVal: {}",ret);
    }catch(json::parse_error & e){
        spdlog::error("{}, {} json : {}",__FILE__,__LINE__, e.what() );
    }

    try{
        //ret = table_insert_row( (char*)tb_sleep,insert_list );
        if ( ret != 0 )
            spdlog::debug("table insert error,retVal: {}",ret);
    }catch(json::parse_error & e){
        spdlog::error("{}, {} json : {}",__FILE__,__LINE__, e.what() );
    }

    //! \brief insert table context
    const char * tb_event = "EVENT_ACTION";
    json event_list =
     {
        {"jsonType",1},
        {
            "add_list",
            {
                {
                    {"file_key_id",2},
                    {"file_name","abc.zip"},
                    {"file_desc","sleep action"},
                    {"file_lib_para",01},
                    {"file_version","1.0.5.6"},
                    {"action_place",100},
                    {"scenery",2},//lib type:1 normal, 2 sleep , 3 event
                    {"role",1},// 1 bac
                    {"run_time",400},// 400 ms
                    {"run_priority",9},
                    {"alive_level",10},
                    {"happiness",3},
                    {"anger",0},
                    {"sadness",0},
                    {"enjoyment",2},
                    {"event_number",1005}
                },
            }
        }, /** \note object create mothed */
    };

    try{
       // ret = table_insert_row((char*)tb_event,event_list);
        if ( ret != 0 )
            spdlog::debug("table insert error,retVal: {}",ret);
    }catch(json::parse_error & e){
        spdlog::error("{}, {} json : {}",__FILE__,__LINE__, e.what() );
    }
    return 0;
}

//!
//! \brief update_role_library::update_role_library_entry
//! \part 1: 检查 table 是否存在,没有就建立表
//! \part 2: 等待网络连接通知,post 获取库同步信息
//! \part 3: 检查 进度监控文件中记录下标值和列表名称add_list\del_list\update_list；以此下标和列表类型继续遍历json文件；
//!          实现断点同步库文件的同步能力,本地库同步信息json中 flag 标识为true 表示云服务器还有库信息待同步。
//! \part 4: 根据进度信息，add_list\del_list\update_list 执行表操作
//! \part 5: 执行 add 、delete 和update 时，向进度监控文件写当前文件 ID 和 时间戳,同步执行完毕清楚文件内容。
//! \part 6: 库未执行完成，角色管理程序无法正常运行。
void update_role_library::update_role_library_entry(void)
{
    const char * post_sava_path = "/userdata/comment/post_in.jscm";
    const char * post_url_base = "http://neuro.coolphper.com/api/ku/kuapi";
    const char * ProgressFile = "/userdata/comment/progressFile.data";
    const char * synchornizeLibraryJson = "/userdata/comment/synchornizeLibraryJson.jscm";

    const char * event_table = "EVENT_ACTION";
    const char * sleep_table = "SLEEP_ACTION";
    const char * normal_table = "NORMAL_ACTION";


    int ret = 0;
    int db_status = 0; //!

    //! \part 1:检查 table 是否存在,没有就建立表
    ret = tableIsExist( (char*)normal_table);
    if ( ret != 0 ){
        ret = CreateNormalTable();
        if (ret != 0){
            spdlog::error("create {} table failure !!! ",normal_table);
            db_status = 1;
        }
    }
    ret = tableIsExist( (char*)sleep_table);
    if(ret != 0 ){
        ret = CreateSleepTable();
        if (ret != 0 ){
            spdlog::error("create {} table failure !!! ",sleep_table);
            db_status += 1;
        }
    }
    ret = tableIsExist( (char*)event_table);
    if(ret != 0 ){
        ret = CreateSleepTable();
        if (ret != 0 ){
            spdlog::error("create {} table failure !!! ",event_table);
            db_status += 1;
        }
    }

    //! \brief query table content
    const char * selectIntSQL = "SELECT * FROM %s;";
    char msg[512];char sql[512];
//    select_result_set resSet;
//    resSet.errmsg = msg;
//    memset(sql,0,sizeof(sql));
//    sprintf(sql,(char*)selectIntSQL,event_table);
//    select_table_sql( (char*)db_name,(char*)sql,&resSet);
//    int index;
//    int i,j;
//    index = resSet.nColumn;
//    for( i = 0; i < resSet.nRow ; i++ )
//    {
//        //spdlog::debug( "第 {} 条记录", i+1 );
//        printf("第 %d 条记录 \n", i+1 );
//        for( j = 0 ; j < resSet.nColumn; j++ )
//        {
//            //spdlog::debug( "字段名: {}  字段值: {} ",  result_set->dbResult[j], result_set->dbResult[index] );
//            printf( "字段名: %s  字段值: %s \n",  resSet.dbResult[j], resSet.dbResult[index] );
//            ++index;
//        }
//        spdlog::debug("-------------------" );
//    }
//    sqlite3_free_table( resSet.dbResult );


    while(1){

        std::vector<uint8_t> rx_buf;
        update_role_library_queue.wait_dequeue(rx_buf);
        spdlog::debug("update role library queue recive data:{}",spdlog::to_hex(rx_buf));
        if (rx_buf.size() == 0)
        {
            spdlog::error("update role library queue recive data:{}",rx_buf.size());
            continue;
        }
        uint8_t buf[1024];
        uint16_t i = 0;
        for (;i<rx_buf.size();i++)
        {
                buf[i]=rx_buf.at(i);
                if(i>1024)
                    break;
        }
        switch (*(uint16_t*)buf)
        {
            case 100://! connect internet notify
            {
                const char * post_field = "{\"version\":\"0.2\",\"page\":1,\"length\":100}";
                bool b = postUrl((char *)post_sava_path, (char*)post_url_base, (char*)post_field);
                if (!b) {
                    spdlog::error("post url {} respose error..",post_url_base);
                    break;
                }
                ret = synchornize_library((char*)synchornizeLibraryJson, (char*)ProgressFile);
                if ( ret != 0 ){
                    spdlog::error("synchornize {} database failure !!! ",synchornizeLibraryJson);
                    db_status = -1;
                }
                //! library synchornize already success
                //! sender signal to user


            }
                break;
            default:
                break;
        }///end switch()

        //! \part
        rx_buf.clear();
        if (db_status != 0 ){
            while (1){
                spdlog::error(" > > > > synchornize database failure!!! ERRORcode={} thread alread paused ...",db_status);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
    }

}

