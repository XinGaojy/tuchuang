package xerrors

type Error struct {
	errCode       string
	errMsg        string
	originalError error
}

func (e *Error) Error() string {
	return e.errMsg
}

func New(errCode, errMsg string, err ...error) error {
	e := &Error{
		errCode: errCode,
		errMsg:  errMsg,
	}
	if len(err) > 0 {
		e.originalError = err[0]
	}
	return e
}

func NewByErr(err error) error {
	return &Error{
		originalError: err,
	}
}
func (e *Error) GetOriginalError() error {
	return e.originalError
}
