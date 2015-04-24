/*
 * MySql.h
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */



#include <my_global.h>
#include <mysql.h>

#if !defined(_MYSQL_H)
#define _MYSQL_H
//---- Functions

void
finish_with_error(MYSQL *con);

void
create_tables(MYSQL *con);

void
store_info(unsigned char * receiveData);

#endif
