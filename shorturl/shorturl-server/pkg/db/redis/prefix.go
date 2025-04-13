package redis

import "strings"

const SERVICEPREFIX = "short_url_"

func GetKey(key string, parts ...string) string {
	key = SERVICEPREFIX + key
	if len(parts) == 0 {
		return key
	}
	key += "_" + strings.Join(parts, "_")
	return key
}
