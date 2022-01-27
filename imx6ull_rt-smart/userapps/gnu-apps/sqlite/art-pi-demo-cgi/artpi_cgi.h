sqlite3* initdb();
int createdb(sqlite3** db);
void conversion_time(struct tm *ptime,char *buff);
int insert_log(sqlite3 *db,char *t,char *l,char *opt,char *detail);
