module("transform", package.seeall)

Transform = {}

function Transform.create(scene,orig)
   local self = {}
   setmetatable(self, {
		   
		   __index = Transform,
		   
		   __tostring = 
		      function(self)
			 return string.format(
			    "%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n",
			    self.arr[1],
			    self.arr[5],
			    self.arr[9],
			    self.arr[13],
			    
			    self.arr[2],
			    self.arr[6],
			    self.arr[10],
			    self.arr[14],
			    
			    self.arr[3],
			    self.arr[7],
			    self.arr[11],
			    self.arr[15],
			    
			    self.arr[4],
			    self.arr[8],
			    self.arr[12],
			    self.arr[16]
			 )
		      end,

		   __mul = 
		      function (self, t)
			 if t.arr then
			    t = t.arr
			 end
			 
			 local res = {
			    0,0,0,0,
			    0,0,0,0,
			    0,0,0,0,
			    0,0,0,0}
			 
			 for x = 1,4 do
			    for y= 1,4 do
			       for a = 1,4 do
				  res[y+4*(x-1)] = res[y+4*(x-1)] + self.arr[y+4*(a-1)]*t[a+4*(x-1)]
			       end
			    end
			 end
			 return Transform.create(res)
		      end
		   


		})
   self.scene=scene
   if orig then
      local arr = orig
      if arr.arr then
	 arr = arr.arr
      end
      self.arr = util.copy(arr)
   else
      self:identity()
   end
   return self
end

function Transform:translate(x,y,z)
   m = {
      1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      x,y,z,1}
--   self.arr[13] = self.arr[13] + x
--   self.arr[14] = self.arr[14] + y
--   self.arr[15] = self.arr[15] + z
   local a = self.arr
--   self.arr=m
   return self:multiply(m)
end

function Transform:rotateX(a)
   c = math.cos(a*math.pi/180)
   s = math.sin(a*math.pi/180)
   m = {
      1,0,0,0,
      0,c,-s,0,
      0,s,c,0,
      0,0,0,1}
   return self:multiply(m)
end

function Transform:rotateY(a)
   c = math.cos(a*math.pi/180)
   s = math.sin(a*math.pi/180)
   m = {
      c,0,s,0,
      0,1,0,0,
      -s,0,c,0,
      0,0,0,1}
   return self:multiply(m)
end

function Transform:rotateZ(a)
   c = math.cos(a*math.pi/180)
   s = math.sin(a*math.pi/180)
   m = {
      c,-s,0,0,
      s,c,0,0,
      0,0,1,0,
      0,0,0,1}
   return self:multiply(m)
end

function Transform:set(v)
   self.arr = util.copy(v)
end

function Transform:identity(v)
   self.arr = {
      1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      0,0,0,1 }
end

function Transform:transform(o)
   o:setTransform(self.scene,unpack(self.arr))
end

function Transform:multiply(t)

   if t.arr then
      t = t.arr
   end
   
   local res = {
      0,0,0,0,
      0,0,0,0,
      0,0,0,0,
      0,0,0,0}
   
   for x = 1,4 do
      for y= 1,4 do
	 for a = 1,4 do
	    res[y+4*(x-1)] = res[y+4*(x-1)] + self.arr[y+4*(a-1)]*t[a+4*(x-1)]
	 end
      end
   end
   self.arr = res
   return self
end


