package mysql

import (
	"database/sql"
	"fmt"
	"shorturl/pkg/config"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

var db *sql.DB

func InitMysql(cnf *config.Config) {
	var err error
	if cnf.Mysql.DSN == "" {
		panic("数据库连接字符串不能为空")
	}
	fmt.Println("cnf.Mysql.DSN: ", cnf.Mysql.DSN)
	db, err = sql.Open("mysql", cnf.Mysql.DSN)
	if err != nil {
		panic(err)
	}
	err = db.Ping()
	if err != nil {
		fmt.Println("err: ", err)
		panic("数据库ping failed")
	}
	db.SetMaxOpenConns(cnf.Mysql.MaxOpenConn)
	db.SetMaxIdleConns(cnf.Mysql.MaxIdleConn)
	db.SetConnMaxLifetime(time.Second * time.Duration(cnf.Mysql.MaxLifeTime))
	fmt.Println("InitMysql ok")
}

func GetDB() *sql.DB {
	return db
}
