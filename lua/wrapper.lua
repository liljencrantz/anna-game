module("wrapper", package.seeall)

function make(definition)

   local myIndex = 
      function(self, name)
	 --print("__index called with " .. name)
	 if definition.getters[name] then
	    return definition.getters[name](self)
	 end
	 if definition.peer_getters[name] then
	    --print("Looking up getter " .. name)
	    --print(self.__peer[name])
	    return self.__peer[name]
	 end
	 if definition.peer_methods[name] then
	    local fun = self.__peer[name]
	    return function (self, ...)
		      return fun(self.__peer, ...)
		   end
	 end
      end

   local myNewindex = 
      function(self, name, value)
	 --print("__newindex called with " .. name)
	 if definition.setters[name] then
	    definition.setters[name](self, value)
	    return
	 end
	 if definition.peer_setters[name] then
	   -- print("Peer setter " .. name)
	    self.__peer[name] = value
	    return
	 end
	 rawset(self, name, value)
	 
      end
   
   definition.create =
      function(...)
	 local res = {}
	 for key, val in pairs(definition) do
	    res[key] = val
	 end
	 res.__peer = 0;
	 setmetatable(res, {
			 __index = myIndex,
			 __newindex = myNewindex})
	 res:constructor(...)
	 return res
      end

   return definition
end

