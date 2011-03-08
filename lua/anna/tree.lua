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


   self.baseTransform = transform.Transform.create(self.world.scene)
   self.fullTransform = transform.Transform.create(self.world.scene)
   self.shearTransform = transform.Transform.create(self.world.scene)
   self.ballTransform = transform.Transform.create(self.world.scene)
   self.baseTransform:translate(
      unpack(pos)
   )
   self.ballTransform:translate(
      0,0,3
   )

--   TreeStem.create(
--      self.world.scene, "tree1"

   table.insert(world.steppable, self)
   
   return self
end

function Tree:step(dt)
   self.shearTransform.arr[9] = 0.1*math.sin(2*self.world.scene.time)
   self.fullTransform:set(self.baseTransform)
   self.fullTransform:multiply(self.shearTransform)
   self.fullTransform:transform(self.stem)

   self.fullTransform:multiply(self.ballTransform)
   self.fullTransform:transform(self.ball)
   
end
   
