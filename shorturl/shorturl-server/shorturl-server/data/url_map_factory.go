package data

import (
	"database/sql"
	"shorturl/pkg/config"
	"shorturl/pkg/log"
)

type IUrlMapDataFactory interface {
	NewUrlMapData(isPublic bool) IUrlMapData
}

type urlMapDataFactory struct {
	log    log.ILogger
	db     *sql.DB
	config *config.Config
}

func NewUrlMapDataFactory(log log.ILogger, db *sql.DB, config *config.Config) IUrlMapDataFactory {
	return &urlMapDataFactory{
		log:    log,
		db:     db,
		config: config,
	}
}
func (f *urlMapDataFactory) NewUrlMapData(isPublic bool) IUrlMapData {
	tableName := "url_map"
	if !isPublic {
		tableName = "url_map_user"
	}
	return newUrlMapData(f.log, f.db, tableName)
}
