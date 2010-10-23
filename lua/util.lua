module("util", package.seeall)

function list(o)
   for key, value in pairs(o) do
      print(key .. ":")
      print(value)
   end
end