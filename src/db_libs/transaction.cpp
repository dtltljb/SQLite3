#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
using namespace std;
void transaction() {
    sqlite3* conn = NULL;
//1. 打开数据库
    int result = sqlite3_open("./mytest.db",&conn);
    if (result != SQLITE_OK) {
        sqlite3_close(conn);
        return;
    }
    const char* createTableSQL ="CREATE TABLE TESTTABLE (int_col INT,float_col REAL, string_col TEXT)";
    sqlite3_stmt* stmt = NULL;
    int len = strlen(createTableSQL);
//2. 准备创建数据表，如果创建失败，需要用sqlite3_finalize释放sqlite3_stmt对象以防止内存泄露。
    if (sqlite3_prepare_v2(conn,createTableSQL,len,&stmt,NULL) != SQLITE_OK) {
        if (stmt)
            sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return;
    }
//3. 通过sqlite3_step命令执行创建表的语句。对于DDL和DML语句而言，sqlite3_step执行正确的返回值
//只有SQLITE_DONE，对于SELECT查询而言，如果有数据返回SQLITE_ROW，当到达结果集末尾时则返回
//SQLITE_DONE。
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return;
    }
//4. 释放创建表语句对象的资源。
    sqlite3_finalize(stmt);
    printf("Succeed to create test table now.\n");
//5. 显式的开启一个事物。
    sqlite3_stmt* stmt2 = NULL;
    const char* beginSQL = "BEGIN TRANSACTION";
    if (sqlite3_prepare_v2(conn,beginSQL,strlen(beginSQL),&stmt2,NULL) !=SQLITE_OK) {
    if (stmt2)
        sqlite3_finalize(stmt2);
    sqlite3_close(conn);
    return;
    }
    if (sqlite3_step(stmt2) != SQLITE_DONE) {
        sqlite3_finalize(stmt2);
        sqlite3_close(conn);
        return;
    }
    sqlite3_finalize(stmt2);
//6. 构建基于绑定变量的插入数据。
    const char* insertSQL = "INSERT INTO TESTTABLE VALUES(?,?,?)";
    sqlite3_stmt* stmt3 = NULL;
    if (sqlite3_prepare_v2(conn,insertSQL,strlen(insertSQL),&stmt3,NULL) !=SQLITE_OK) {
        if (stmt3)
            sqlite3_finalize(stmt3);
        sqlite3_close(conn);
        return;
        }

/// @note 7. 基于已有的SQL语句，迭代的绑定不同的变量数据
    int insertCount = 10;
    const char* strData = "This is a test.";

    for (int i = 0; i < insertCount; ++i) {
        ///> @note 在绑定时，最左面的变量索引值是1。
        sqlite3_bind_int(stmt3,1,i);
        sqlite3_bind_double(stmt3,2,i * 1.0);
        sqlite3_bind_text(stmt3,3,strData,strlen(strData),SQLITE_TRANSIENT);
        if (sqlite3_step(stmt3) != SQLITE_DONE) {
            sqlite3_finalize(stmt3);
            sqlite3_close(conn);
            return;
        }
        //重新初始化该sqlite3_stmt对象绑定的变量。
        sqlite3_reset(stmt3);
        printf("Insert Succeed.\n");
    }
    sqlite3_finalize(stmt3);

///> @note 8. 提交之前的事物。
    const char* commitSQL = "COMMIT";
    sqlite3_stmt* stmt4 = NULL;
    if (sqlite3_prepare_v2(conn,commitSQL,strlen(commitSQL),&stmt4,NULL) !=SQLITE_OK) {
        if (stmt4)
        sqlite3_finalize(stmt4);
        sqlite3_close(conn);
        return;
    }
    if (sqlite3_step(stmt4) != SQLITE_DONE) {
        sqlite3_finalize(stmt4);
        sqlite3_close(conn);
        return;
    }
    sqlite3_finalize(stmt4);
//9. 为了方便下一次测试运行，我们这里需要删除该函数创建的数据表，否则在下次运行时将无法
//创建该表，因为它已经存在。
    const char* dropSQL = "DROP TABLE TESTTABLE";
    sqlite3_stmt* stmt5 = NULL;
    if (sqlite3_prepare_v2(conn,dropSQL,strlen(dropSQL),&stmt5,NULL) != SQLITE_OK)
    {
        if (stmt5)
            sqlite3_finalize(stmt5);
        sqlite3_close(conn);
        return;
    }
    if (sqlite3_step(stmt5) == SQLITE_DONE) {
        printf("The test table has been dropped.\n");
        }
        sqlite3_finalize(stmt5);
        sqlite3_close(conn);
}
