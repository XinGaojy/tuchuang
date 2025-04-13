package config

import (
	"github.com/fsnotify/fsnotify"
	"github.com/spf13/viper"
	_ "github.com/spf13/viper/remote"
	"log"
	"time"
)

type Config struct {
	Http struct {
		IP   string `mapstructure:"ip"`
		Port int    `mapstructure:"port"`
	}
	Cos struct {
		BucketUrl string `mapstructure:"buketUrl"`
		SecretID  string `mapstructure:"secretId"`
		SecretKey string `mapstructure:"secretKey"`
		CDNDomain string `mapstructure:"cdnDomain"`
	} `mapstructure:"cos"`
	DependOn struct {
		ShortUrl struct {
			Address     string
			AccessToken string
		}
	} `mapstructure:"dependOn"`
	Log struct {
		Level   string
		LogPath string `mapstructure:"logPath"`
	}
}

var conf *Config

// 从文件初始化配置文件
func InitConfig(filepath string, typ ...string) {
	v := viper.New()
	v.SetConfigFile(filepath)
	if len(typ) > 0 {
		v.SetConfigType(typ[0])
	}
	err := v.ReadInConfig()
	if err != nil {
		log.Fatal(err)
	}
	conf = &Config{}
	err = v.Unmarshal(conf)
	if err != nil {
		log.Fatal(err)
	}
	// 配置热更新
	v.OnConfigChange(func(in fsnotify.Event) {
		v.Unmarshal(conf)
	})
	v.WatchConfig()
}

// 从etcd初始化配置文件
func InitConfigFromEtcd(etcdAddr, key, typ string) {
	v := viper.New()
	v.AddRemoteProvider("etcd3", etcdAddr, key)
	v.SetConfigType(typ)
	err := v.ReadRemoteConfig()
	if err != nil {
		log.Fatal(err)
	}
	conf = &Config{}
	err = v.Unmarshal(conf)
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		for {
			<-time.After(time.Second * 3)
			err = v.WatchRemoteConfig()
			if err != nil {
				log.Println(err)
				continue
			}
			err = v.Unmarshal(conf)
			if err != nil {
				log.Println(err)
				continue
			}
		}
	}()
}

func GetConfig() *Config {
	return conf
}
