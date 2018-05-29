/*
 * sqlite.h
 *
 *  Created on: 28 may. 2018
 *      Author: lpastor
 */

#ifndef SQLITE_SQLITE_H_
#define SQLITE_SQLITE_H_
#include <common/common.h>


namespace std {

static char queryAnswer[500];
static bool resultCallBack = false;

class sqlite {

private:

	static int queryResult(void *nothing, int argc, char **argv, char **colName);
	sqlite3 *db;
	int response;
	char *error;
	char dataBaseName[100];
	bool openOK;

public:
	sqlite(const char *dataBaseName);
	int init(void);
	int insert(const char *tableName, const char *data);
	int deleteItem(const char *tableName, const char *columName, const char *data);
	int deletAllItems(const char *tableName);
	char* selectFirstItem(const char *columName, const char *tableName);
	int createDataBase(const char *nameDB, const char *dbColummsAndFormats);
	char* countItems(const char *tableName);
	void close(void);
	virtual ~sqlite();
};

} /* namespace std */

#endif /* SRC_SQLITE_SQLITE_H_ */
