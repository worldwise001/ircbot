#include "../../libcircle/circle.h"
#include <db.h>

#define DB_HOME "./db"
#define DB_MASTER "circle"

DB_ENV * db_env;
DB * master;

void construct(IRCQ * ircq) {
    int db_err;
    db_env = NULL;
    master = NULL;

    if (mkdir(DB_HOME, 0755)) {
        if (errno != EEXIST) return;
    }

    db_err = db_env_create(&db_env, 0);
    if (db_err) {
        db_env = NULL;
        return;
    }

    db_env->set_lg_dir(db_env, "logs");
    db_env->set_data_dir(db_env, "data");
    db_env->set_errfile(db_env, ircq->__ircenv->__ircerr);
    db_err = db_env->open(db_env, DB_HOME, DB_INIT_CDB | DB_INIT_MPOOL | DB_CREATE | DB_THREAD, 0);
    if (db_err) {
        db_env->remove(db_env, DB_HOME, 0);
        db_env = NULL;
        return;
    }

    db_err = db_create(&master, db_env, 0);
    if (db_err) {
        master = NULL;
        db_env->close(db_env, 0);
        db_env->remove(db_env, DB_HOME, 0);
        db_env = NULL;
        return;
    }

    db_err = master->open(master, DB_MASTER, NULL, DB_HASH, DB_CREATE | DB_THREAD, 0);
    if (db_err) {
        master->close(master, 0);
        master = NULL;
        db_env->close(db_env, 0);
        db_env->remove(db_env, DB_HOME, 0);
        db_env = NULL;
        return;
    }
}

void destruct(IRCQ * ircq) {
    db_env->remove(db_env, DB_HOME, 0);
}

void db_open(const char * name, DB_TYPE type) {
    DB* db = NULL;
    int db_err;

    if (master == NULL || db_env == NULL) return;
    db_err = db_create(db, db_env, 0);

    db->open(db, name, NULL, )
}

void db_close(const char * name);

DB * db_get(const char * name) {
    
}

void db_store(const char * name, void * key, size_t keylen, void * value, size_t valuelen);
void db_value(const char * name, void * key, size_t keylen);

