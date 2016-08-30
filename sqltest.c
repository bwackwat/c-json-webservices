#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

int main(){
        SQLHENV env;
        SQLHDBC dbc;
        long res;

	char driver[256];
	char attr[256];
	SQLSMALLINT driver_ret;
	SQLSMALLINT attr_ret;
	SQLUSMALLINT direction;
	SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

	direction = SQL_FETCH_FIRST;
	while(SQL_SUCCEEDED(ret = SQLDrivers(env, direction,
	driver, sizeof(driver), &driver_ret,
	attr, sizeof(attr), &attr_ret))) {
		direction = SQL_FETCH_NEXT;
		printf("%s - %s\n", driver, attr);
		if (ret == SQL_SUCCESS_WITH_INFO)
			printf("\tdata truncation\n");
	}

        SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
        SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
        SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

        res = SQLConnect(dbc, (SQLCHAR*)"localhost", SQL_NTS,
                (SQLCHAR*)"postgres", SQL_NTS,
                (SQLCHAR*)"abc123", SQL_NTS);
        printf("RES: %i\n", res);

        SQLDisconnect(dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
        SQLFreeHandle(SQL_HANDLE_ENV, env);

        printf("\n");
        return 0;
}
