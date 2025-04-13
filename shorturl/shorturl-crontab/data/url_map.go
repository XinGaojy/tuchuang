package data

import (
	"database/sql"
	"fmt"
)

type data struct {
	db *sql.DB
}

func NewData(db *sql.DB) *data {
	return &data{
		db: db,
	}
}

func (d *data) GetMaxID(tableName string) (int64, error) {
	sqlStr := fmt.Sprintf("select max(id) as id from %s", tableName)
	var id sql.NullInt64
	row := d.db.QueryRow(sqlStr)
	err := row.Scan(&id)
	if err != nil && err != sql.ErrNoRows {
		return 0, err
	}
	if id.Valid {
		return id.Int64, nil
	} else {
		return 0, nil
	}
}
