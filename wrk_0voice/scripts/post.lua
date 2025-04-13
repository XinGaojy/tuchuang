-- example HTTP POST script which demonstrates setting the
-- HTTP method, body, and adding a header

-- wrk.method = "POST"
-- wrk.body   = "{"email": "472251823@qq.com", "firstPwd": "e10adc3949ba59abbe56e057f20f883e", "nickName": "lucky", "phone": "18612345678", "userName": "qingfu"}"
-- wrk.headers["Content-Type"] = "application/json"
function random(n, m)
    math.randomseed(os.clock()*math.random(1000000,90000000)+math.random(1000000,90000000))
    return math.random(n, m)
end

function randomLetter(len)
    local rt = ""
    for i = 1, len, 1 do
            rt = rt..string.char(random(97,122))
    end
    return rt
end

local request_body = {
    email = "472251823@qq.com",
    firstPwd = "e10adc3949ba59abbe56e057f20f883e",
    nickName = randomLetter(15),
    phone = "18612345678",
    userName = randomLetter(15)
}
print(request_body)



request = function()
    print("req")
    wrk.method = "POST"
    -- wrk.body = '{"username":"admin","password":"123456"}'
    wrk.body   = '{"email": "472251823@qq.com", "firstPwd": "e10adc3949ba59abbe56e057f20f883e", "nickName": "lucky", "phone": "18612345678", "userName": "qingfu"}'
    wrk.headers["Content-Type"] = "application/json"
    return  wrk.format(wrk.method,"/api/reg",wrk.headers, wrk.body)
end
response = function(status, headers, body)
    print(body) --调试用，正式测试时需要关闭，因为解析response非常消耗资源
end