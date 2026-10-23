#include <mysql/field_types.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

#define DB_SERVER_IP        "127.0.0.1"
#define DB_SERVER_PORT      3306
#define DB_USERNAME         "admin"
#define DB_PASSWORD         "0904"
#define SQL_INSERT_TBL_USER "INSERT TBL_USER(U_NAME, U_GENDER) VALUES('eric', 'male');"

#define DB_DEFAULT_DB       "KING_DB"
#define FILE_IMAGE_LENGTH   1000000

int king_mysql_select(MYSQL *handle) {
    const char select_query[] = "SELECT * FROM TBL_USER;";

    // mysql_real_query --> sql
    if (mysql_real_query(handle, select_query, strlen(select_query))) {
        printf("mysql_real_query: %s\n", mysql_error(handle));
        return -1;
    }
    // store -->
    MYSQL_RES *res = mysql_store_result(handle);
    if (res == NULL) {
        printf("mysql_store_result: %s\n", mysql_error(handle));
        return -2;
    }

    // rows/fields
    int rows = mysql_num_rows(res);
    printf("rows: %d\n", rows);

    int fields = mysql_num_fields(res);
    printf("fields: %d\n", rows);

    //fetch
    MYSQL_ROW row;

    while((row = mysql_fetch_row(res))) {
        int i = 0;
        for (i = 0; i < fields; i++) {
            printf("%s\t", row[i]);
        }

        printf("\n");
    }


    mysql_free_result(res);
    return 0;
}

// filename: path + filename
// buffer: store 
int read_image(char *filename, char *buffer) {
    if (filename == NULL || buffer == NULL) return -1;
    FILE *fp = fopen(filename, "rb"); 
    if (fp == NULL) {
        printf("fopen failed\n");
        return -2;
    }

    // 讲指针移到文件最末尾，通过偏移量就能知道文件大小
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp); // file size
    fseek(fp, 0, SEEK_SET);

    printf("file length: %d\n", length);
    int size = fread(buffer, 1, length, fp); // 从fb中,每次读1个byte，读 length 次到buffer中
    printf("size length: %d\n", size);
    if (size != length) {
        printf("fread failed\n");
        return -3;
    }

    fclose(fp);
    return size;
}


int write_image(char *filename, char *buffer, int length) {
    if (filename == NULL || buffer == NULL || length <= 0) {
        return -1;
    }
    FILE *fp = fopen(filename, "wb+"); 
    if (fp == NULL) {
        printf("fopen failed\n");
        return -2;
    }

    int size = fwrite(buffer, 1, length, fp);
    if (size != length) {
        printf("fwrite failed: %d\n", size);
        return -3;
    }

    fclose(fp);

    return size;
}

int mysql_write(MYSQL *handle, char *buffer, int length) {
    if (handle == NULL || buffer == NULL || length < 0) return -1;

    const char INSERT_IMAGE_USER[] = "INSERT TBL_USER(U_NAME, U_GENDER, U_IMG) VALUES('Kelley', 'female', ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(handle);
    int ret = mysql_stmt_prepare(stmt, INSERT_IMAGE_USER, strlen(INSERT_IMAGE_USER));

    if (ret) {
        printf("mysql_stmt_prepare: %s\n", mysql_error(handle));
        return -2;
    }

    MYSQL_BIND param = {0};
    param.buffer_type = MYSQL_TYPE_LONG_BLOB;
    param.buffer = NULL;
    param.is_null = 0;
    param.length = NULL;

    ret = mysql_stmt_bind_param(stmt, &param);
    if (ret) {
        printf("mysql stmt bind param: %s\n", mysql_error(handle));
        return -3;
    }

    ret = mysql_stmt_send_long_data(stmt, 0, buffer, length);
    if (ret) {
        printf("mysql_stmt_prepare: %s\n", mysql_error(handle));
        return -4;
    }

    ret = mysql_stmt_execute(stmt);
    if (ret) {
        printf("mysql_stmt_execute: %s\n", mysql_error(handle));
        return -5;
    }

    ret = mysql_stmt_close(stmt);

    return 0;
}


int mysql_read(MYSQL *handle, char *buffer, int length) {

    const char SQL_SELECT_IMG_USER[] = "SELECT U_IMG FROM TBL_USER WHERE U_NAME='Kelley';";

    if (handle == NULL || buffer == NULL || length < 0) return -1;

    MYSQL_STMT *stmt = mysql_stmt_init(handle);
    int ret = mysql_stmt_prepare(stmt, SQL_SELECT_IMG_USER, strlen(SQL_SELECT_IMG_USER));

    if (ret) {
        printf("mysql_stmt_prepare: %s\n", mysql_error(handle));
        return -2;
    }

    MYSQL_BIND result = {0};
    result.buffer_type = MYSQL_TYPE_LONG_BLOB;
    unsigned long total_length = 0;
    result.length = &total_length;

    ret = mysql_stmt_bind_result(stmt, &result);
    if (ret) {
        printf("mysql_stmt_bind_result: %s \n", mysql_error(handle));
    }

    ret = mysql_stmt_execute(stmt);
    if (ret) {
        printf("mysql_stmt_execute: %s\n", mysql_error(handle));
        return -4;
    }

    ret = mysql_stmt_store_result(stmt);
    if (ret) {
        printf("mysql_stmt_store_result: %s\n", mysql_error(handle));
        return -5;
    }

    while(1) {
        mysql_stmt_fetch(stmt);
        if (ret != 0 && ret != MYSQL_DATA_TRUNCATED) break;

        int start = 0;
        while(start < (int)total_length) {
            result.buffer = buffer + start;
            result.buffer_length = 1;
            ret = mysql_stmt_fetch_column(stmt, &result, 0, start);
            start += result.buffer_length;
        }
    }

    mysql_stmt_close(stmt);

    return total_length;
}

int main() {


    MYSQL mysql;
    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init: %s\n", mysql_error(&mysql));
        return -1;
    }

    if(!mysql_real_connect(&mysql, DB_SERVER_IP, DB_USERNAME, 
                       DB_PASSWORD, DB_DEFAULT_DB, DB_SERVER_PORT, NULL, 0)) {

        printf("mysql_real_connect: %s\n", mysql_error(&mysql));
        return -2;
    }

    // insert
    // if (mysql_real_query(&mysql, SQL_INSERT_TBL_USER, strlen(SQL_INSERT_TBL_USER))) {
    //     printf("mysql_real_query: %s\n", mysql_error(&mysql));
    // }

    // select
 
    king_mysql_select(&mysql);

    printf("case: mysql --> read image and write mysql\n");
    char buffer[FILE_IMAGE_LENGTH] = {0};
    int length = read_image("mysql_image.jpg", buffer);

    mysql_write(&mysql, buffer, length);

    if (length < 0) {
        mysql_close(&mysql);
        return 0;
    }

    printf("case: mysql --> read mysql and write image\n");
    memset(buffer, 0, FILE_IMAGE_LENGTH);
    length = mysql_read(&mysql, buffer, FILE_IMAGE_LENGTH);

    write_image("a.jpg", buffer, length);


    mysql_close(&mysql);

    return 0;
}
