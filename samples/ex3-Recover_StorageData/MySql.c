/*
 * MySql.h
 *
 *
 *  Created by Alvaro Rodrigo Alonso Bivou.
 *
 *
 */

#include "MySql.h"

void
finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

void
create_tables(MYSQL *con)
{
	//---- Create a new table every time
	if (mysql_query(con, "DROP TABLE IF EXISTS servidores")) {finish_with_error(con);}
	//---- Table Servidores
	if (mysql_query(con,"CREATE TABLE IF NOT EXISTS servidores (\
		  id int(11) NOT NULL AUTO_INCREMENT,\
		  nombre varchar(30) NOT NULL,\
		  PRIMARY KEY (id)\
		) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=5")){finish_with_error(con);}
	//---- Table Sensores
	if (mysql_query(con,"CREATE TABLE IF NOT EXISTS sensores (\
		  id int(11) NOT NULL AUTO_INCREMENT,\
		  id_servidor int(11) NOT NULL,\
		  nombre varchar(30) NOT NULL,\
		  PRIMARY KEY (id)\
		) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13")){finish_with_error(con);}
	//---- Table Temperaturas
	if (mysql_query(con, "CREATE TABLE IF NOT EXISTS temperaturas ( \
	            id int(11) NOT NULL AUTO_INCREMENT,\
				id_sensor int(11) NOT NULL,\
				fecha timestamp NOT NULL,\
				temp float NOT NULL,\
				PRIMARY KEY (id)\
			) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=74")) {finish_with_error(con);}
}

void
store_info(unsigned char * receiveData)
{
	int prefix, value0,value1,value2,value3;

	switch(receiveData[0]){
	case 'S':
		sscanf(receiveData,"S:%d:%d:",prefix,value0);
		store_sensors_info(prefix, value0);
		break;
	case 'T':
		sscanf(receiveData,"T:d%:%d:%d:%d:%d",prefix,value0,value1,value2, value3);
		store_temp_info(prefix,value0,value1,value2, value3);
		break;
	}
	return;
}
