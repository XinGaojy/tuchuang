package utils

import "regexp"

func IsUrl(url string) bool {
	return true
	pattern := `^(http|https)://[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(?:/[^/]*)*$`
	regExp := regexp.MustCompile(pattern)
	return regExp.MatchString(url)
}
