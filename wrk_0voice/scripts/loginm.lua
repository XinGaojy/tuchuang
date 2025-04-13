-- example HTTP POST script which demonstrates setting the
-- HTTP method, body, and adding a header

request = function()
    -- print("req")
    wrk.method = "POST"
    -- 用户名 Darren；密码：这里是123456做md5的结果
    wrk.body = '{"user":"qingfu","pwd":"e10adc3949ba59abbe56e057f20f883e"}'
    wrk.headers["Content-Type"] = "application/json"
    return  wrk.format(wrk.method,"/api/loginm",wrk.headers, wrk.body)
end
response = function(status, headers, body)
    -- print(body) --调试用，正式测试时需要关闭，因为解析response非常消耗资源
end
