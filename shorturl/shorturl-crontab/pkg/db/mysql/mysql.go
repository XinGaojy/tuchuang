package mysql

import (
	"database/sql"
	_ "github.com/go-sql-driver/mysql"
	"shorturl-crontab/pkg/config"
	"time"
)

var db *sql.DB

func InitMysql(cnf *config.Config) {
	var err error
	if cnf.Mysql.DSN == "" {
		panic("数据库连接字符串不能为空")
	}
	db, err = sql.Open("mysql", cnf.Mysql.DSN)
	if err != nil {
		panic(err)
	}
	db.SetMaxOpenConns(cnf.Mysql.MaxOpenConn)
	db.SetMaxIdleConns(cnf.Mysql.MaxIdleConn)
	db.SetConnMaxLifetime(time.Second * time.Duration(cnf.Mysql.MaxLifeTime))
}

func GetDB() *sql.DB {
	return db
}
