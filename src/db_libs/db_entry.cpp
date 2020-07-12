//thirty libs
#include <nlohmann/json.hpp>
#include "blockingconcurrentqueue.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

#include "table_opration.h"
#include "select.h"

const char * event_table = "EVENT_ACTION";
const char * sleep_table = "SLEEP_ACTION";
const char * normal_table = "NORMAL_ACTION";

int db_entry(void)
{
    //! \part 1:检查 table 是否存在,没有就建立表
    int db_status = 0;
    ret = tableIsExist( (char*)normal_table);
    if ( ret != 0 ){
        ret = CreateNormalTable();
        if (ret != 0){
            spdlog::error("create {} table failure !!! ",normal_table);
            db_status = 1;
        }
    }

    /**
     * @part
     */

    int row = 23;
    std::map<std::string,int> result;
    ret = by_mid_query_name_int_time(row,result);
    if( result.size() == 1 ){
        spdlog::info(" {}  {} ",result.iterator[0],result.iterator[1]);
    }
}

