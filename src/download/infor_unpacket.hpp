#ifndef INFOR_UNPACKET_HPP
#define INFOR_UNPACKET_HPP

// download status
#define DOWNLOAD_ING        3
#define DOWNLOAD_FAILUR     2
#define DOWNLOAD_SUCCESS    1

// version status
#define _DIFF_VERSION_FLAG   (int)1
#define _SAME_VERSION_FLAG   (int)0
// update result
#define UPDATE_RESULT_FALT  3
#define UPDATE_RESULT_ING   2
#define UPDATE_RESULT_OKAY  1
// update type
#define UPDATE_FORCE    1       //强制升级
#define AGREE_UPDATE    1       //用户同意升级
//
#define USER_AGREE_FLAG 1


// UI notify
typedef struct {
    uint16_t holder_place;
    uint8_t result;			// =1 联网成功
}cmd_linkNet_notify;

// CORE 请求 UI Version
typedef struct {
    uint8_t result;			// =1 请求
}cmd_ui_version;
// UI 应答 CORE
typedef struct {
    uint16_t holder_place;      //! cmd
    uint8_t Ver[18];         // 参考版本管理办法 ASSIC CODE
}cmd_ui_version_r;

// UI 请求 CORE total Version
typedef struct {
    uint8_t result;		// =1 请求
}cmd_ui_to_core_total_version;

//! CORE 应答 UI
typedef struct{
    uint8_t	device_type;
    uint8_t	Ver[18];
    uint16_t    desc_length;		//版本描述信息长度
    uint8_t     desc[1024];            //版本描述信息，最大1024
}sub_version_format;

typedef struct {
    uint8_t  status;                        // 0 success, 1 failuer ,file don't exist
    uint8_t  sub_version_total;
    sub_version_format	sebVersion[128];   // 参考 struct{ }sub_version_format

}cmd_core_to_ui_total_version;

//!CORE 请求 UI upgrade brain
typedef struct {
    uint8_t status;             //update mode: =1 force ,2 suggest.
    uint8_t device_type;        //固件类型:1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t Ver[18];             // 总版本号
    uint16_t desc_length;		//版本描述信息长度
    uint8_t desc[1024];		   //版本描述信息，最大1024
}cmd_update_request;            // brain upgrade request

// UI 应答 CORE upgrade brain
typedef struct {
    uint16_t holder_place;
    uint8_t device_type;            //type:1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t result;			// 1 同意,2 放弃升级
}cmd_update_confirm;


//CORE 请求 UI upgrade braiin
typedef struct {
    uint8_t status;             //update mode: =1 force ,2 suggest.
    uint8_t device_type;        //固件类型:1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t Ver[18];             // 总版本号
    uint16_t desc_length;		//版本描述信息长度
    uint8_t desc[1024];		   //版本描述信息，最大1024
}comment_upgrade_request;           // joint upgrade request

// UI 应答 CORE upgrade brain
typedef struct {
    uint16_t holder_place;
    uint8_t device_type;            //type:1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t result;			// 1 同意,2 放弃升级
}comment_upgrade_confirm;           // confirm : joint upgrade request

// CORE notify UI
typedef struct {
    //uint16_t holder_place;
    uint8_t device_type;        //type: 1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t progress;		//range: 0~100
    uint8_t result;		// 1升级完成,2升级中
}cmd_update_progress;
// CORE notify UI
typedef struct {
    //uint16_t holder_place;
    uint8_t device_type;        //!type:1 brain update,2 brain UI ,3 brain core,4 joint. etc....
    uint8_t progress;		//!rang: 0~100
    uint8_t result;		// 1下载成功,2 失败 3 下载中...
}cmd_download_progress;

// CORE notify UI reboot
typedef struct {
   uint8_t result;			//! : 1 upgrade success after reboot, 2 upgrade comment failure
}cmd_core_to_ui_reboot;


#define DESC_INFO_MAX_SIZE  sizeof(cmd_update_request)
#define	MSG_DATA_MAX_SIZE	DESC_INFO_MAX_SIZE

typedef struct {
        uint16_t msg_type;
        uint16_t index;
        uint16_t error_code;
        uint16_t len_of_msg;
        uint8_t	 data[MSG_DATA_MAX_SIZE];
}mq_msg_format;



/*unpacket recive message */
void cmd_ui_version_resp(uint16_t length,char *buf);

/* packet send information message*/
void cmd_update_progress_ui(cmd_update_progress *msg);
void cmd_download_progress_ui(cmd_download_progress *msg);
void cmd_get_version_to_ui(cmd_ui_version *msg);
void cmd_download_request_ui(cmd_update_request *msg);
void cmd_upgrade_request_ui(cmd_update_request *msg);
void cmd_upgrade_comment_request_ui(comment_upgrade_request *msg);
void cmd_download_request_to_ui(cmd_update_request *msg);
void cmd_upgrade_comment_progress_ui(cmd_update_progress *msg);
void cmd_upgrade_reboot_notify_ui(cmd_core_to_ui_reboot *msg);
void cmd_version_unchange_notify_ui(void);
void cmd_connect_failure_notify_ui(void);

#endif // INFOR_UNPACKET_HPP
