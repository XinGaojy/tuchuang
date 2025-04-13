package utils

import (
	"strings"
)

// 字母的顺序可以适当调整，增加安全性。这里是62字符
const chars = "klmOPQRnopqrs012wxyz34FGHIJK5tuABCDEv67TUVW89abhijLMNScdefgXYZ"

// 把自增id转成62进制字符串
func ToBase62(num int64) string {
	result := ""
	for num > 0 {
		result = string(chars[num%62]) + result
		num /= 62
	}
	return result
}

// 根据短链key查找自增id，再通过这个自增id从redis查找对应的长链，如果redis查找不到就到mysql查找。
func ToBase10(str string) int64 {
	var res int64 = 0
	for _, s := range str {
		index := strings.IndexRune(chars, s)
		res = res*62 + int64(index)
	}
	return res
}
