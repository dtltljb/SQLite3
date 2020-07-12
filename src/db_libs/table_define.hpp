#ifndef TABLE_DEFINE_HPP
#define TABLE_DEFINE_HPP

#include <string>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <map>
#include <list>

typedef struct {
    int id;
    std::string name;
    std::string desc;
    int para;
    int version;
    int place;
    int mode;
    int role;
    int rTime;
    int priority;
    int scenery;
    int type;
    int sequ;
    std::string url;
    int timeStamp;
}file_table_s;

typedef struct{
    int id;
    std::string name;
    std::string desc;
    int para;
    int type;       ///> 1、明确过渡; 2、确认过渡；
    int role;
    int priority;
    int eventNum; ///> role + eventNum
} event_para_s;

typedef struct{
    int id;
    std::string name;
    std::string desc;
    int para;
    int type;  /// story_line_player_rule_e
    int mode;
    int role;
    int rule;
} story_line_s;

typedef struct{
    int id;
    std::string name;
    int para;
    int file_id;
    int mode;
    int role;
    int type;    ///> 1 事件反馈、2 过渡内容
    int eventNum;///> role + file_id + eventNum
} event_answer_s;

///> 角色role, 工作模式 mode, 内容类型 scenery 定义在robot.hpp文件
///
typedef enum{
    zip_type = 1,
    json_type = 2,  ///json 语句
} file_type_define_e;

typedef enum{
    FILE_SCENERY_NORMAL = 1,
    FILE_SCENERY_CLEAR_TRANSIT = 2,
    FILE_SCENERY_ENTER_TRANSIT = 3,
    FILE_SCENERY_EVENT_ANSWER = 4,
} file_scenery_define_e;

typedef enum{
    EVENT_PARA_CLEAR_TRANSIT = 1,
    EVENT_PARA_ENTER_TRANSIT = 2,
} event_para_type_e;

typedef enum{
    ASSIGN_METHOD = 0,
    RANDOM_METHOD =1,
}story_line_select_e;

typedef enum{
    random_mode = 1,
    sequence_mode =2,
} story_line_player_rule_e;

typedef enum{    
    EVENT_ANSWER_TYPE_ANSWER = 1,
    EVENT_ANSWER_TYPE_TRANSIT = 2,
} event_answer_type_e;


typedef enum{
    UNKOWN = 0,
    NORMAL_STORY = 1 ,
    ASK_ANSWER_STORY =2 ,
    WOOD_MAN_STORY =3 ,
}story_line_id_e;

#endif // TABLE_DEFINE_HPP
