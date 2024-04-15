-- Rime Script >https://github.com/baopaau/rime-lua-collection/blob/master/calculator_translator.lua
-- 简易计算器（执行任何Lua表达式）
--
-- 格式：=<exp>
-- Lambda语法糖：\<arg>.<exp>|
--
-- 例子：
-- =1+1 输出 2
-- =floor(9^(8/7)*cos(deg(6))) 输出 -3
-- =e^pi>pi^e 输出 true
-- =max({1,7,2}) 输出 7
-- =map({1,2,3},\x.x^2|) 输出 {1, 4, 9}
-- =map(range(-5,5),\x.x*pi/4|,deriv(sin)) 输出 {-0.7071, -1, -0.7071, 0, 0.7071, 1, 0.7071, 0, -0.7071, -1}
-- =$(range(-5,5,0.01))(map,\x.-60*x^2-16*x+20|)(max)() 输出 21.066
-- =test(\x.trunc(sin(x),1e-3)==trunc(deriv(cos)(x),1e-3)|,range(-2,2,0.1)) 输出 true
--
-- 安装：
-- - 将本文件保存至 <rime>/lua/
-- - 在 <rime>/rime.lua 新增一行：
--   `calculator_translator = require("calculator_translator")`
-- - 在 <rime>/<schema>.schema.yaml 新增：
--   `engine/translators/@next: lua_translator@calculator_translator`
--   `recognizer/patterns/expression: "^=.*$"`
-- 注：
-- - <rime> 替换为RIME的共享目录
-- - <schema> 替换为自己的方案ID
-- - 如目录／文件不存在，请自行创建

-- 定义全局函数、常数（注意命名空间污染）
cos = math.cos
sin = math.sin
tan = math.tan
acos = math.acos
asin = math.asin
atan = math.atan
rad = math.rad
deg = math.deg

abs = math.abs
floor = math.floor
ceil = math.ceil
mod = math.fmod
trunc = function (x, dc)
  if dc == nil then
    return math.modf(x)
  end
  return x - mod(x, dc)
end

round = function (x, dc)
  dc = dc or 1
  local dif = mod(x, dc)
  if abs(dif) > dc / 2 then
    return x < 0 and x - dif - dc or x - dif + dc
  end
  return x - dif
end

random = math.random
randomseed = math.randomseed

inf = math.huge
MAX_INT = math.maxinteger
MIN_INT = math.mininteger
pi = math.pi
sqrt = math.sqrt
exp = math.exp
e = exp(1)
ln = math.log
log = function (x, base)
  base = base or 10
  return ln(x)/ln(base)
end

min = function (arr)
  local m = inf
  for k, x in ipairs(arr) do
   m = x < m and x or m
  end
  return m
end

max = function (arr)
  local m = -inf
  for k, x in ipairs(arr) do
   m = x > m and x or m
  end
  return m
end

sum = function (t)
  local acc = 0
  for k,v in ipairs(t) do
    acc = acc + v
  end
  return acc
end

avg = function (t)
  return sum(t) / #t
end

isinteger = function (x)
  return math.fmod(x, 1) == 0
end

-- iterator . array
array = function (...)
  local arr = {}
  for v in ... do
    arr[#arr + 1] = v
  end
  return arr
end

-- iterator <- [form, to)
irange = function (from, to, step)
  if to == nil then
    to = from
    from = 0
  end
  step = step or 1
  local i = from - step
  to = to - step
  return function()
    if i < to then
      i = i + step
      return i
    end
  end
end

-- array <- [form, to)
range = function (from, to, step)
  return array(irange(from, to, step))
end

-- array . reversed iterator
irev = function (arr)
  local i = #arr + 1
  return function()
    if i > 1 then
      i = i - 1
      return arr[i]
    end
  end
end

-- array . reversed array
arev = function (arr)
  return array(irev(arr))
end

test = function (f, t)
  for k,v in ipairs(t) do
    if not f(v) then
      return false
    end
  end
  return true
end

-- # Functional
map = function (t, ...)
  local ta = {}
  for k,v in pairs(t) do
    local tmp = v
    for _,f in pairs({...}) do tmp = f(tmp) end
    ta[k] = tmp
  end
  return ta
end

filter = function (t, ...)
  local ta = {}
  local i = 1
  for k,v in pairs(t) do
    local erase = false
    for _,f in pairs({...}) do
      if not f(v) then
        erase = true
        break
      end
    end
    if not erase then
	  ta[i] = v
	  i = i + 1
    end
  end
  return ta
end

-- e.g: foldr({2,3},\n,x.x^n|,2) = 81
foldr = function (t, f, acc)
  for k,v in pairs(t) do
    acc = f(acc, v)
  end
  return acc
end

-- e.g: foldl({2,3},\n,x.x^n|,2) = 512
foldl = function (t, f, acc)
  for v in irev(t) do
    acc = f(acc, v)
  end
  return acc
end

-- 调用链生成函数（HOF for method chaining）
-- e.g: chain(range(-5,5))(map,\x.x/5|)(map,sin)(map,\x.e^x*10|)(map,floor)()
--    = floor(map(map(map(range(-5,5),\x.x/5|),sin),\x.e^x*10|))
--    = {4, 4, 5, 6, 8, 10, 12, 14, 17, 20}
-- 可以用 $ 代替 chain
chain = function (t)
  local ta = t
  local function cf(f, ...)
    if f ~= nil then
      ta = f(ta, ...)
      return cf
    else
      return ta
    end
  end
  return cf
end

-- # Statistics
fac = function (n)
  local acc = 1
  for i = 2,n do
    acc = acc * i
  end
  return acc
end

nPr = function (n, r)
  return fac(n) / fac(n - r)
end

nCr = function (n, r)
  return nPr(n,r) / fac(r)
end

MSE = function (t)
  local ss = 0
  local s = 0
  local n = #t
  for k,v in ipairs(t) do
    ss = ss + v*v
    s = s + v
  end
  return sqrt((n*ss - s*s) / (n*n))
end

-- # Linear Algebra


-- # Calculus
-- Linear approximation
lapproxd = function (f, delta)
  local delta = delta or 1e-8
  return function (x)
           return (f(x+delta) - f(x)) / delta
         end
end

-- Symmetric approximation
sapproxd = function (f, delta)
  local delta = delta or 1e-8
  return function (x)
           return (f(x+delta) - f(x-delta)) / delta / 2
         end
end

-- 近似导数
deriv = function (f, delta, dc)
  dc = dc or 1e-4
  local fd = sapproxd(f, delta)
  return function (x)
           return round(fd(x), dc)
         end
end

-- Trapezoidal rule
trapzo = function (f, a, b, n)
  local dif = b - a
  local acc = 0
  for i = 1, n-1 do
    acc = acc + f(a + dif * (i/n))
  end
  acc = acc * 2 + f(a) + f(b)
  acc = acc * dif / n / 2
  return acc
end

-- 近似积分
integ = function (f, delta, dc)
  delta = delta or 1e-4
  dc = dc or 1e-4
  return function (a, b)
           if b == nil then
             b = a
             a = 0
           end
           local n = round(abs(b - a) / delta)
           return round(trapzo(f, a, b, n), dc)
         end
end

-- Runge-Kutta
rk4 = function (f, timestep)
  local timestep = timestep or 0.01
  return function (start_x, start_y, time)
           local x = start_x
           local y = start_y
           local t = time
           -- loop until i >= t
           for i = 0, t, timestep do
             local k1 = f(x, y)
             local k2 = f(x + (timestep/2), y + (timestep/2)*k1)
             local k3 = f(x + (timestep/2), y + (timestep/2)*k2)
             local k4 = f(x + timestep, y + timestep*k3)
             y = y + (timestep/6)*(k1 + 2*k2 + 2*k3 + k4)
             x = x + timestep
           end
           return y
         end
end


-- # System
date = os.date
time = os.time
path = function ()
  return debug.getinfo(1).source:match("@?(.*/)")
end


local function serialize(obj)
  local type = type(obj)
  if type == "number" then
    return isinteger(obj) and floor(obj) or obj
  elseif type == "boolean" then
    return tostring(obj)
  elseif type == "string" then
    return '"'..obj..'"'
  elseif type == "table" then
    local str = "{"
    local i = 1
    for k, v in pairs(obj) do
      if i ~= k then  
        str = str.."["..serialize(k).."]="
      end
      str = str..serialize(v)..", "  
      i = i + 1
    end
    str = str:len() > 3 and str:sub(0,-3) or str
    return str.."}"
  elseif pcall(obj) then -- function类型
    return "callable"
  end
  return obj
end

-- greedy：随时求值（每次变化都会求值，否则结尾为特定字符时求值）
local greedy = true

local function calculator(input, seg)
  if string.sub(input, 1, 1) ~= "=" then return end
  
  local expfin = greedy or string.sub(input, -1, -1) == ";"
  local exp = (greedy or not expfin) and string.sub(input, 2, -1) or string.sub(input, 2, -2)
  
  -- 空格输入可能
  exp = exp:gsub("#", " ")
  
  yield(Candidate("number", seg.start, seg._end, exp, "表达式"))
       
  if not expfin then return end
  
  local expe = exp
  -- 链式调用语法糖
  expe = expe:gsub("%$", " chain ")
  -- lambda语法糖
  do
    local count
    repeat
      expe, count = expe:gsub("\\%s*([%a%d%s,_]-)%s*%.(.-)|", " (function (%1) return %2 end) ")
    until count == 0
  end
  --yield(Candidate("number", seg.start, seg._end, expe, "展开"))
  
  -- 防止危险操作，禁用os和io命名空间
  if expe:find("i?os?%.") then return end
  -- return语句保证了只有合法的Lua表达式才可执行
  local s, result = pcall(load("return "..expe))
  if not s then return end
  if result == nil then return end
  
  result = serialize(result)
  yield(Candidate("number", seg.start, seg._end, result, "答案"))
  yield(Candidate("number", seg.start, seg._end, exp.." = "..result, "等式"))
end

return calculator