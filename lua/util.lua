module("util", package.seeall)

function list(o)
   for key, value in pairs(o) do
      print(key .. ":")
      print(value)
   end
end

function pack(...)
  return arg
end

function dot(a, b)
   local res=0
   for i, v in ipairs(a) do
      res = res + v*b[i]
   end
   return res
end
