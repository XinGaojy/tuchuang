package utils

import (
	"fmt"
	"math"
	"math/rand"
	"testing"
)

func TestToBase62(t *testing.T) {
	for i := 0; i < 1000; i++ {
		d := rand.Int63n(math.MaxInt64)
		str := ToBase62(d)
		d1 := ToBase10(str)
		fmt.Println(d, d1, str)
		if d != d1 {
			t.Error("base62 转换失败")
		}
	}
}
