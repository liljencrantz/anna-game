module("scene", package.seeall)

require("anna")
require("wrapper")

CAMERA_DISTANCE = 2.0

Scene = wrapper.make(
   {
      constructor = 
	 function(self, levels, size)
	    self.__peer = anna.ScenePeer.create(levels, size)
	    self.__player = 1
	    self.active = true
	 end,
      
      peer_getters = {
	 size=0,
	 time=0,
	 renderQuality=0,
	 cameraAngle=0
      },
      
      peer_setters = {
	 size=0,
	 time=0,
	 renderQuality=0,
	 cameraAngle=0
      },
      
      peer_methods = {
	 render=0,
	 getHeight=0,
	 getRealTime=0
      },

      getters = {
	 player =
	    function (self)
	       return self.__player
	    end,
	 cameraPos =
	    function (self)
	       return {self.__peer.cameraX,
		       self.__peer.cameraY,
		       self.__peer.cameraZ}
	    end
      },
      
      setters = {
	 player = 
	    function (self,p)
	       self.__peer.player = p.__peer
	       self.__player = p
	       self.cameraAngle = p.angle
	    end,
	 cameraPos = 
	    function (self, pos)
	       self.__peer.cameraX = pos[1]
	       self.__peer.cameraY = pos[2]
	       self.__peer.cameraZ = pos[3]
	    end
      }
   })

function Scene:step(dt)
   self.time = self.time + dt
   self.player:step(self, dt)
   
   acs = 1.0 - 0.3*dt
   
   if self.player.actions.walk_forward then
      ca = self.cameraAngle
      pa = self.player.angle
      if pa > ca+180 then
	 ca = acs*ca + (1.0-acs)*(pa-360)
	 if ca < 0.0 then
	    ca = ca + 360
	 end
      elseif pa < ca-180 then
	 ca = acs*ca + (1.0-acs)*(pa+360)
	 if ca > 360.0 then
	    ca = ca - 360
	 end
      else
	 ca = acs*ca + (1.0-acs)*pa
      end
      self.cameraAngle = ca
   end
   
   pos = self.player.pos
   pos[1] = pos[1] - CAMERA_DISTANCE * math.cos(self.cameraAngle*math.pi/180)
   pos[2] = pos[2] - CAMERA_DISTANCE * math.sin(self.cameraAngle*math.pi/180)
   pos[3] = pos[3] + 5
   self.cameraPos = pos

end


