module("world", package.seeall)

CAMERA_DISTANCE = 5.0

World = {}

function World.create(name, load)
   local self = {}
   setmetatable(self, {__index=World})

   self.scene = anna.Scene.create(name, load)
   self.active = true
   
   self.scene.cameraX = 40
   self.scene.cameraY = 40
   self.scene.cameraZ = 500

   self.steppable = {}

   return self
end

function World:step(dt)
   self.scene.time = self.scene.time + dt
   acs = 1.0 - 0.3*dt

   for _,item in ipairs(self.steppable) do
      item:step(dt)
   end

   if self.player then
      
      
      if self.player.actions.walk_forward then
	 ca = self.scene.cameraAngle
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
	 self.scene.cameraAngle = ca
      end
      --self.scene.cameraAngle = self.player.angle
      --self.scene.cameraAngle = 0
      
      local tpos = {self.player.pos[1], self.player.pos[2], self.player.pos[3]}
      tpos[1] = tpos[1] - CAMERA_DISTANCE * math.cos(self.scene.cameraAngle*math.pi/180)
      tpos[2] = tpos[2] - CAMERA_DISTANCE * math.sin(self.scene.cameraAngle*math.pi/180)
      tpos[3] = math.max(tpos[3],self.scene:getHeight(tpos[1],tpos[2])) + 8
      self.scene.cameraX = self.scene.cameraX*0.9 + tpos[1]*0.1
      self.scene.cameraY = self.scene.cameraY*0.9 + tpos[2]*0.1
      self.scene.cameraZ = self.scene.cameraZ*0.9 + tpos[3]*0.1
      
   end

end


