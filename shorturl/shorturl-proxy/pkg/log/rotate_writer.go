package log

import (
	"gopkg.in/natefinch/lumberjack.v2"
	"io"
	"sync"
)

type fileRotateWriter struct {
	data map[string]io.Writer
	sync.RWMutex
}

func (frw *fileRotateWriter) getWriter(logPath string) io.Writer {
	frw.RLock()
	defer frw.RUnlock()
	w, ok := frw.data[logPath]
	if !ok {
		return nil
	}
	return w
}
func (frw *fileRotateWriter) setWriter(logPath string, w io.Writer) io.Writer {
	frw.Lock()
	defer frw.Unlock()
	frw.data[logPath] = w
	return w
}

var _fileRotateWriter *fileRotateWriter

func init() {
	_fileRotateWriter = &fileRotateWriter{
		data: map[string]io.Writer{},
	}
}

func GetRotateWriter(logPath string) io.Writer {
	if logPath == "" {
		panic("日志文件路径不能为空")
	}
	writer := _fileRotateWriter.getWriter(logPath)
	if writer != nil {
		return writer
	}
	writer = &lumberjack.Logger{
		//文件名
		Filename: logPath,
		//单个文件大小单位MB
		MaxSize: 1,
		//最多保留文件数
		MaxBackups: 15,
		//最长保留时间（天）
		MaxAge:    7,
		LocalTime: true,
	}
	return _fileRotateWriter.setWriter(logPath, writer)
}
