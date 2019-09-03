#pragma once

#include<mysql/mysql.h>
#include<cstdio>
#include<cstdlib>
#include<jsoncpp/json/json.h>

namespace image_system
{  
static MYSQL* MySQLInit()	
{
    //1.创建一个mysql的句柄
    MYSQL* mysql=mysql_init(NULL);
    //2.与数据库建立连接
    if(mysql_real_connect(mysql,"127.0.0.1","root","","image_system",3306,NULL,0)==NULL)
	{
	     //数据库连接失败
	     printf("连接失败！%s\n",mysql_error(mysql));
	     return NULL;
	}
    //3.设置编码格式
    mysql_set_character_set(mysql,"utf8");
    return mysql;
}

static void MySQLRelease(MYSQL* mysql)
{
    mysql_close(mysql);
}
//此处Insert等操作，函数依赖的输入信息比较多
//为了防止参数太多，可以使用JSON来封装参数
class ImageTable
{
public:
    ImageTable(MYSQL* mysql)
	 :_mysql(mysql)
    {}

    bool Insert(const Json::Value& image)
    {
	char sql[4*1024]={0};
	sprintf(sql,"insert into image_table values(null,'%s',%d,'%s','%s','%s','%s')",
		image["image_name"].asCString(),image["size"].asInt(),image["upload_time"].asCString(),
		image["md5"].asCString(),image["type"].asCString(),image["path"].asCString());
	printf("[Insert sql] %s\n",sql);

	int ret=mysql_query(_mysql,sql);
	if(ret!=0)
	{
	    printf("Insert 执行失败 %s\n",mysql_error(_mysql));
	    return false;
	}
	return true;
    }

    bool SelectAll(Json::Value* images)
    {
	char sql[1024*4]={0};
	sprintf(sql,"select * from image_table");
	int ret=mysql_query(_mysql,sql);
	if(ret!=0)
	{
	    printf("SelectAll 执行失败 %s\n",mysql_error(_mysql));
	    return false;
	}
	//遍历结果集合，并把结果集合写到images参数之中
	MYSQL_RES* result=mysql_store_result(_mysql);
    int rows=mysql_num_rows(result);
    for(int i=0;i<rows;++i)
    {
        MYSQL_ROW row=mysql_fetch_row(result);
        //数据库查出的每条记录都相当于是一个图片的信息
        //需要把这个信息转成JSON格式
        Json::Value image;
        image["image_id"]=atoi(row[0]);
        image["image_name"]=row[1];
        image["size"]=atoi(row[2]);
        image["upload_time"]=row[3];
        image["md5"]=row[4];
        image["type"]=row[5];
        image["path"]=row[6];
        images->append(image);
    }
	mysql_free_result(result);
	return true;
    }

    bool SelectOne(int image_id,Json::Value* image_ptr)
    {
        char sql[1024*4]={0};
        sprintf(sql,"select * from image_table where image_id=%d",image_id);
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("SelectOne 执行SQL失败%s\n",mysql_error(_mysql));
            return false;
        }
        //遍历结果集合
        MYSQL_RES* result=mysql_store_result(_mysql);
        int rows=mysql_num_rows(result);
        if(rows!=1)
        {
            printf("SelectOne查询结果不是1条记录，实际查询到%d条！\n",rows);
        }
        MYSQL_ROW row=mysql_fetch_row(result);
        Json::Value image;
        image["image_id"]=atoi(row[0]);
        image["image_name"]=row[1];
        image["size"]=atoi(row[2]);
        image["upload_time"]=row[3];
        image["md5"]=row[4];
        image["type"]=row[5];
        image["path"]=row[6];
        *image_ptr=image;
        mysql_free_result(result);

        return true;
    }

    bool Delete(int image_id)
    {
        char sql[1024*4]={0};
        sprintf(sql,"delete from image_table where image_id=%d",image_id);
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("Delete 执行SQL失败%s\n",mysql_error(_mysql));
            return false;
        }
        return true;
    }
private:
    MYSQL* _mysql;
};
}
