package data

import (
	"database/sql"
	"fmt"
	"shorturl/pkg/log"
	"time"
)

type IUrlMapData interface {
	GenerateId(createAt, updateAt int64) (int64, error)
	Update(e UrlMapEntity) error
	GetByID(id int64) (UrlMapEntity, error)
	GetByOriginal(originalUrl string) (UrlMapEntity, error)
	GetAll() ([]UrlMapEntity, error)
	IncrementTimes(id int64, incrementTimes int, now time.Time) error
}
type UrlMapEntity struct {
	ID          int64
	ShortKey    string
	OriginalUrl string
	Times       string
	CreateAt    int64
	UpdateAt    int64
}

func newUrlMapData(logger log.ILogger, db *sql.DB, tableName string) IUrlMapData {
	return &urlMapData{
		log:       logger,
		db:        db,
		tableName: tableName,
	}
}

type urlMapData struct {
	log       log.ILogger
	db        *sql.DB
	tableName string
}

func (d *urlMapData) GenerateId(createAt, updateAt int64) (int64, error) {
	sqlStr := fmt.Sprintf("insert into %s (create_at,update_at)values(?,?);", d.tableName)
	res, err := d.db.Exec(sqlStr, createAt, updateAt)
	if err != nil {
		d.log.Error(err)
		return 0, err
	}
	return res.LastInsertId()
}

func (d *urlMapData) Update(e UrlMapEntity) error {
	sqlStr := fmt.Sprintf("update %s set  short_key=?,original_url=?,update_at=? where id=?", d.tableName)
	_, err := d.db.Exec(sqlStr, e.ShortKey, e.OriginalUrl, e.UpdateAt, e.ID)
	if err != nil {
		d.log.Error(err)
		return err
	}
	return nil
}
func (d *urlMapData) GetByID(id int64) (UrlMapEntity, error) {
	sqlStr := fmt.Sprintf("select original_url from %s where id = ?;", d.tableName)
	row := d.db.QueryRow(sqlStr, id)
	entity := UrlMapEntity{}
	var originalUrl sql.NullString
	err := row.Scan(&originalUrl)
	if err != nil && err != sql.ErrNoRows {
		d.log.Error(err)
		return entity, err
	}
	if originalUrl.Valid {
		entity.OriginalUrl = originalUrl.String
	}
	return entity, nil
}

func (d *urlMapData) GetByOriginal(originalUrl string) (UrlMapEntity, error) {
	sqlStr := fmt.Sprintf("select id,short_key from %s where original_url = ?;", d.tableName)
	row := d.db.QueryRow(sqlStr, originalUrl)
	entity := UrlMapEntity{}
	var shortKey sql.NullString
	err := row.Scan(&entity.ID, &shortKey)
	if err != nil && err != sql.ErrNoRows {
		d.log.Error(err)
		return entity, err
	}
	if shortKey.Valid {
		entity.ShortKey = shortKey.String
	}
	return entity, nil

}
func (d *urlMapData) GetAll() ([]UrlMapEntity, error) {
	sqlStr := fmt.Sprintf("select id, short_key,original_url from %s;", d.tableName)
	rows, err := d.db.Query(sqlStr)
	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		d.log.Error(err)
		return nil, err
	}
	list := make([]UrlMapEntity, 0)
	for rows.Next() {
		e := UrlMapEntity{}
		var originalUrl sql.NullString
		var shortKey sql.NullString
		err = rows.Scan(&e.ID, &shortKey, &originalUrl)
		if err != nil {
			log.Error(err)
			return nil, err
		}
		if originalUrl.Valid {
			e.OriginalUrl = originalUrl.String
		}
		if shortKey.Valid {
			e.ShortKey = shortKey.String
		}
		list = append(list, e)
	}
	return list, nil
}

// 统计短链访问次数
func (d *urlMapData) IncrementTimes(id int64, incrementTimes int, now time.Time) error {
	sqlStr := fmt.Sprintf("update %s set  times = times + ?,update_at=? where id=?", d.tableName)
	_, err := d.db.Exec(sqlStr, incrementTimes, now.Unix(), id)
	if err != nil {
		d.log.Error(err)
		return err
	}
	return nil
}
