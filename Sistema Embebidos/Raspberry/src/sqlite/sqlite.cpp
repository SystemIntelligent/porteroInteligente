/*
 * sqlite.cpp
 *
 *  Created on: 28 may. 2018
 *      Author: lpastor
 */

#include <sqlite/sqlite.h>

#define PRINT_DEBUG_SQLITE(x,y)                  cout << x << y << endl;

namespace std {

char sqlite::queryAnswer[500];
bool sqlite::resultCallBack;


int sqlite::queryResult(void *nothing, int argc, char **argv,
		char **colName) {

	strcpy(queryAnswer, argv[0]);
	PRINT_DEBUG_SQLITE("sqlite(query result): ", queryAnswer);
	resultCallBack = true;
	return 0;
}

sqlite::sqlite(const char *dataBaseName) {
	strcpy(this->dataBaseName, dataBaseName);
	error = NULL;
	response = 0;
	db = NULL;
	openOK = false;
	resultCallBack = false;
	memset(queryAnswer,'\0',sizeof(queryAnswer));
}
int sqlite::init(void) {
	PRINT_DEBUG_SQLITE("sqlite(DB Name): ",this->dataBaseName);
	response = sqlite3_open(this->dataBaseName, &db);
	if (response!=SQLITE_OK) {
		PRINT_DEBUG_SQLITE("sqlite: ", "No se pudo abrir la DB");
		openOK = false;
		sqlite3_close(db);
		return 1;
	}
	resultCallBack = false;
	openOK = true;
	return 0;
}

int sqlite::insert(const char *tableName, const char *data) {
#define QUERY_INSERT_TEMPLATE "INSERT INTO %s VALUES ('%s')"
	if (openOK == false) {
		return 1;
	}
	char query[500];
	sprintf(query, QUERY_INSERT_TEMPLATE, tableName, data);
	PRINT_DEBUG_SQLITE("sqlite(insert): ", query);
	response = sqlite3_exec(db, query, NULL, 0, &error);
	if (response != SQLITE_OK) {
		PRINT_DEBUG_SQLITE("sqlite: ", "Error al insertar item en DB");
		sqlite3_free(error);
		sqlite3_close(db);
		return 1;
	}

	return 0;
}

int sqlite::deleteItem(const char *tableName, const char *columName,
		const char *data) {
	if (openOK == false) {
		return 1;
	}
#define QUERY_DELETE_TEMPLATE "DELETE FROM %s WHERE %s='%s'"
	char query[500];
	sprintf(query, QUERY_DELETE_TEMPLATE, tableName, columName, data);
	PRINT_DEBUG_SQLITE("sqlite(delete item): ", query);
	response = sqlite3_exec(db, query, NULL, 0, &error);
	if (response != SQLITE_OK) {
		PRINT_DEBUG_SQLITE("sqlite: ", "Error al eliminar item en DB");
		sqlite3_free(error);
		sqlite3_close(db);
		return 1;
	}

	return 0;
}
int sqlite::deletAllItems(const char *tableName) {
	if (openOK == false) {
		return 1;
	}
#define QUERY_DELETE_ALL_ITEMS_TEMPLATE "DELETE FROM '%s'"
	char query[500];
	sprintf(query, QUERY_DELETE_ALL_ITEMS_TEMPLATE, tableName);
	PRINT_DEBUG_SQLITE("sqlite(delete all items): ", query);
	response = sqlite3_exec(db, query, NULL, 0, &error);
	if (response != SQLITE_OK) {
		PRINT_DEBUG_SQLITE("sqlite: ", "Error al eliminar todos los items de una tabla ");
		sqlite3_free(error);
		sqlite3_close(db);
		return 1;
	}

	return 0;

}
int sqlite::createDataBase(const char *nameDB, const  char *dbColummsAndFormats) {
	if (openOK == false) {
		return 1;
	}
#define QUERY_CREATE_TABLE "sqlite3 /root/%s 'CREATE TABLE %s %s;'"
	char cmdSystem[500];
	sprintf(cmdSystem, QUERY_CREATE_TABLE, nameDB, dbColummsAndFormats);
	PRINT_DEBUG_SQLITE("sqlite(create table): ", cmdSystem);
	system(cmdSystem);
	return 0;
}
void sqlite::close(void) {
	sqlite3_close(db);
}
char* sqlite::selectFirstItem(const char *columName, const char *tableName) {
	if (openOK == false) {
		return NULL;
	}

	#define QUERY_SELECT_FIRST_ITEM_TEMPLATE "SELECT %s FROM %s LIMIT 1"
	char query[500];
		sprintf(query, QUERY_SELECT_FIRST_ITEM_TEMPLATE, columName, tableName);
		PRINT_DEBUG_SQLITE("sqlite(select first item): ", query);
		resultCallBack = false;
		response = sqlite3_exec(db, query, queryResult, 0, &error);
		if (response != SQLITE_OK) {
			PRINT_DEBUG_SQLITE("sqlite: ", "Error 'select first item'");
			sqlite3_free(error);
			sqlite3_close(db);
			return NULL;
		}
        unsigned long tInit = millis();
        while((millis()-tInit) < 1000 && resultCallBack == false);
        if (resultCallBack == true){
        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "ok");
        	return queryAnswer;
        }
        else{
        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "error");
            return NULL;
        }

}

char* sqlite::selectItem(const char *column,const char *whereCondition,const char *columnToShow, const char *tableName){
	if (openOK == false) {
			return NULL;
		}

		#define QUERY_SELECT_ITEM_TEMPLATE "SELECT %s FROM %s WHERE %s = '%s'"
		char query[500];
			sprintf(query, QUERY_SELECT_ITEM_TEMPLATE, columnToShow, tableName,column,whereCondition);
			PRINT_DEBUG_SQLITE("sqlite(select item): ", query);
			resultCallBack = false;
			response = sqlite3_exec(db, query, queryResult, 0, &error);
			if (response != SQLITE_OK) {
				PRINT_DEBUG_SQLITE("sqlite: ", "Error 'select first item'");
				sqlite3_free(error);
				sqlite3_close(db);
				return NULL;
			}
	        unsigned long tInit = millis();
	        while((millis()-tInit) < 1000 && resultCallBack == false);
	        if (resultCallBack == true){
	        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "ok");
	        	return queryAnswer;
	        }
	        else{
	        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "error");
	            return NULL;
	        }

}

char* sqlite::countItems(const char *tableName) {
	if (openOK == false) {
		return NULL;
	}

	#define QUERY_COUNT_ITEMS_TEMPLATE "SELECT COUNT(*) FROM %s"
	char query[500];
		sprintf(query, QUERY_COUNT_ITEMS_TEMPLATE, tableName);
		PRINT_DEBUG_SQLITE("sqlite(count item): ", query);
		resultCallBack = false;
		response = sqlite3_exec(db, query, queryResult, 0, &error);
		if (response != SQLITE_OK) {
			PRINT_DEBUG_SQLITE("sqlite: ", "Error 'count items'");
			sqlite3_free(error);
			sqlite3_close(db);
			return NULL;
		}
        unsigned long tInit = millis();
        while((millis()-tInit) < 1000 && resultCallBack == false);
        if (resultCallBack == true){
        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "ok");
        	return queryAnswer;
        }
        else{
        	PRINT_DEBUG_SQLITE("sqlite(result query-->select): ", "error");
            return NULL;
        }

}

sqlite::~sqlite() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
