module("tree", package.seeall)

Tree = {}

function Tree.create(world,name,scale, pos)
   local self = {}
   setmetatable(self, {__index = Tree})
   
   self.angle = angle
   self.pos=pos
   self.scale=scale
   self.world = world
   
   self.stem = TreeStem.create(
      self.world.scene,
      "tree1", 
      scale)

   self.ball = BallPeer.create(
      self.world.scene,
      "ball1", 
      scale)


   self.transform = transform.Transform.create(self.world.scene)
   self.transform:translate(
      unpack(pos)
   )
   self.transform:transform(self.stem)
   self.transform:transform(self.ball)

--   TreeStem.create(
--      self.world.scene, "tree1"

   table.insert(world.steppable, self)
   
   return self
end

function Tree:step(dt)

end
   
