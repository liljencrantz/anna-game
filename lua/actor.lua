module("actor", package.seeall)

animations={
   run= 
      function(actor)
	 local phase = math.sin(actor.world.scene.time*6)
	 local phase2 = math.sin(actor.world.scene.time*12)
	 return {
	    leftLeg={0, 60*phase, 0},
	    rightLeg={0, -60*phase, 0},
	    leftArm={0, -50*phase, 0},
	    rightArm={0, 50*phase, 0},
	    torso={0 * phase2, 10-5*phase2, 0},
	    head={0 * phase2, 15-5*phase2, 0},
	 }
      end,
   idle= 
      function(actor)
	 local phase = math.sin(actor.world.scene.time)
	 return {
	    leftLeg={0,0,0},
	    rightLeg={0,0,0},
	    leftArm={0,0,0},
	    rightArm={0,0,0},
	    torso={0, 0, 3*phase},
	    head={0,0,3*phase}
	 }
      end
}

Actor = {}

function Actor.create(world,name)
   local self = {}
   setmetatable(self, {__index = Actor})
   
   self.actions = {}
   self.world=world
   self.walkSpeed = 8.0
   self.reactionSpeed = 10.0
   self.turnSpeed = 250.0
   self.walkSpeedReverse = 3.0
   self.angle = 0
   self:animationInit()
   self.movementAnimation=animations.idle
   self.actionAnimation={}
   self.allAnimations={}
   self.experience = {}
   self.vel={0,0,0}
   self.pos={0,0,0}

   table.insert(world.steppable, self)
   
   return self
end

function Actor:addExperience(type, amount)
   self.experience[type] = amount + (self.experience[type] or 0)
end

function Actor:getExperience(type)
   return self.experience[type] or 0
end

function Actor:setMovementAnimation(a)
   if a then
      self.movementAnimation=animations[a]
   else
      self.movementAnimation=animations.idle
   end
end

function Actor:stepAllAnimations(dt)

   local TRANSITION_SPEED = math.min(8*dt,0.3)

   local res = {}
   local had_movement = false
   for _, data in ipairs(self.allAnimations) do
      if data.animation == self.movementAnimation then
	 data.weight = data.weight + TRANSITION_SPEED
	 had_movement = true
      else
	 data.weight = data.weight - TRANSITION_SPEED
      end
      if data.weight > 1 then
	 data.weight=1
      end
      if data.weight > 0 then
	 table.insert(res, data)
      end
   end
   if not had_movement then
      table.insert(res, {weight=TRANSITION_SPEED, animation=self.movementAnimation})
   end

   self.allAnimations = res
end

function Actor:step(dt)
   if self.actions.turn_left then
      self.angle = self.angle + dt*self.turnSpeed
      if self.angle > 360 then
	 self.angle = self.angle-360
      end
   end
   if self.actions.turn_right then
      
      self.angle = self.angle - dt*self.turnSpeed
      if self.angle < 0 then
	 self.angle = self.angle+360
      end
   end
   
   local vel = self.vel
   local velFactor = self.reactionSpeed*dt

   local slope_x, slope_y = self.world.scene:getSlope(self.pos[1], self.pos[2])
   local slope = {slope_x, slope_y}
--   local slope = util.pack(self.world.scene:getSlope(self.pos[1], self.pos[2]))
   local dir={math.cos(self.angle*math.pi/180), math.sin(self.angle*math.pi/180)}

   local mySlope = util.dot(slope, dir)
	      
   local slopeFactor = 1/(1+1.5*mySlope*mySlope + 0.5*mySlope)
--   print(string.format("Slope is %.2f, slopeFactor is %.2f\n", mySlope, slopeFactor));

   if self.actions.walk_forward then
      local vt = {
	 math.cos(self.angle*math.pi/180)*self.walkSpeed,
	 math.sin(self.angle*math.pi/180)*self.walkSpeed
      }

      vel[1] = vel[1]*(1.0-velFactor) + vt[1]*velFactor*slopeFactor
      vel[2] = vel[2]*(1.0-velFactor) + vt[2]*velFactor*slopeFactor
     
      self:setMovementAnimation("run")
   elseif self.actions.walk_backward then
      local vt = {
	 -math.cos(self.angle*math.pi/180)*self.walkSpeedReverse,
	 -math.sin(self.angle*math.pi/180)*self.walkSpeedReverse
      }

      vel[1] = vel[1]*(1.0-velFactor) + vt[1]*velFactor*slopeFactor
      vel[2] = vel[2]*(1.0-velFactor) + vt[2]*velFactor*slopeFactor
   else
      vel[1] = (1.0-velFactor)*vel[1]
      vel[2] = (1.0-velFactor)*vel[2]
      self:setMovementAnimation("idle")
   end

   local pos = self.pos
--   local pos = {
--      self.pos[1],
--      self.pos[2],
--      self.pos[3]}
   pos[1] = pos[1]+vel[1]*dt
   pos[2] = pos[2]+vel[2]*dt
   pos[3] = self.world.scene:getHeight(pos[1], pos[2])
   
   pos[1] = math.min(math.max(pos[1], 30), self.world.scene.size-30)
   pos[2] = math.min(math.max(pos[2], 30), self.world.scene.size-30)
   
--   self.pos = pos
   
   self:stepAllAnimations(dt)
   self:animate()
end

function Actor:animationInit()
   self.body = {
      torso = {
	 ball=anna.BallPeer.create(
	    self.world.scene, "torso1",0.6),
	 roffset={0,0,0},	 
	 offset={0,0,1.2}	 

      },
      head = {
	 ball=anna.BallPeer.create(
	    self.world.scene, "head1",0.5),
	 roffset={0,0,0.8},
	 offset={0,0,1.2}	 

      },
      leftLeg = {
	 ball = anna.BallPeer.create(
	    self.world.scene, "leftLeg1",0.4),
	 roffset={0,0,-0.5},
	 offset={0.0,-0.25,0.5}
      },

      rightLeg = {
	 ball = anna.BallPeer.create(
	    self.world.scene, "rightLeg1",0.4),
	 roffset={0.0,0,-0.5},
	 offset={0.0,0.25,0.5}
      },
      leftArm = {
	 ball = anna.BallPeer.create(
	    self.world.scene, "leftArm1",0.3),
	 roffset={0,0,-0.4},
	 offset={0,-0.6,1.5}
      },

      rightArm = {
	 ball = anna.BallPeer.create(
	    self.world.scene, "rightArm1",0.3),
	 roffset={0.0,0,-0.4},
	 offset={0,0.6,1.5}
      }
   }

   for partName, partData in pairs(self.body) do
      local a={0,0,0}
      off = self.body[partName].roffset
      self.body[partName].ball:setOffset(
	 self.world.scene, unpack(off))
   end      

   
end

function Actor:animate()
   local angles={}
   
   for _, a in pairs(self.allAnimations) do
      table.insert(angles,{weight=a.weight, angle=a.animation(self)})
   end
   
   local f1 = math.cos(self.angle*math.pi/180)
   local f2 = math.sin(self.angle*math.pi/180)
   
   for partName, partData in pairs(self.body) do
      local a={0,0,0}
      local factor = 0
--      print(partName)
      for _, data in ipairs(angles) do
--	 print("Available angles:")
--	 util.list(angle)
	 if data.angle[partName] then
	    a[1] = a[1] + data.angle[partName][1] *data.weight 
	    a[2] = a[2] + data.angle[partName][2] *data.weight 
	    a[3] = a[3] + data.angle[partName][3]*data.weight 
	    factor = factor + data.weight
--	    print(data.weight)
	 end
      end
      for i = 1,2,3 do
	 a[i] = a[i]/math.max(1,factor)
      end
--      print(factor)
      local off = self.body[partName].offset
      local x = off[1]*f1 - off[2]*f2
      local y = off[1]*f2 + off[2]*f1

      self.body[partName].ball:setLocation(
	 self.world.scene, 
	 self.pos[1]+x, self.pos[2]+y, self.pos[3]+off[3], 
	 self.angle+a[1],a[2],a[3])

   end
end
   
