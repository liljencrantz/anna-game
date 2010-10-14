module("actor", package.seeall)

require("anna")
require("wrapper")
require("math")

Actor = wrapper.make(
   {
      getters = {
	 pos = 
	    function (self)

	       return {
		  self.__peer.posX,
		  self.__peer.posY,
		  self.__peer.posZ
	       }
	    end
      },
      peer_setters={angle=0},
      peer_getters={angle=0},
      peer_methods={},
      setters = {
	 pos =
	    function (self, vec)
	       self.__peer.posX=vec[1]
	       self.__peer.posY=vec[2]
	       self.__peer.posZ=vec[3]
	    end
      },
   
   })

function Actor:constructor(name)
   self.__peer  = anna.ActorPeer.create(name)
   self.actions = {}

   self.walkSpeed = 8.0
   self.turnSpeed = 250.0
   self.walkSpeedReverse = 3.0
   self.angle = 0
end

function Actor:step(scene, dt)
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
   
   if self.actions.walk_forward then
      local pos = self.pos
      pos[1] = pos[1]+math.cos(self.angle*math.pi/180)*dt*self.walkSpeed
      pos[2] = pos[2]+math.sin(self.angle*math.pi/180)*dt*self.walkSpeed
      pos[3] = scene:getHeight(pos[1], pos[2])
      self.pos = pos
   end
   if self.actions.walk_backward then
      local pos = self.pos
      
      pos[1] = pos[1]-math.cos(self.angle*math.pi/180)*dt*self.walkSpeedReverse
      pos[2] = pos[2]-math.sin(self.angle*math.pi/180)*dt*self.walkSpeedReverse
      pos[3] = scene:getHeight(pos[1], pos[2])
      self.pos = pos
   end
end

