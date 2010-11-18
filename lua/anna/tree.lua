module("tree", package.seeall)

Tree = {}

function Tree.create(world,name,angle, scale, pos)
   local self = {}
   setmetatable(self, {__index = Tree})
   
   self.angle = angle
   self.pos=pos
   self.scale=scale

   table.insert(world.steppable, self)
   
   return self
end

function Tree:step(dt)

end
   
