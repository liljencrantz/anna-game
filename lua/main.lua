module("anna", package.seeall)

require("lua/util")
require("lua/world")
require("lua/actor")
require("lua/input")
require("math")
require("lua/editor")
require("lua/transform")

function initWorld()

   local w = world.World.create("anna", true)
   w.player = actor.Actor.create(w, "Bosse");
   
   actor.Actor.create(w, "Hasse");
   
   boid = BoidSetPeer.create(w.scene, 40, 40, 40);
   
   for i = 11, 110, 12 do
      for j = 11, 110, 12 do
	 local x = i+5*math.sin(0.1*j)
	 local y = j+5*math.sin(0.1*i)
	 local h = w.scene:getTerrainElement(10,x,y) or 1000
	 
	 if h < 10 and (i/40)%2 >= 1 then
	    local b = BallPeer.create(w.scene, "ball1", 1.8);
	    b:setLocation(w.scene, x,y,h+1.1,0,0,0);	    
	 end
      end
   end

   return w
end

function run()

   if false then
      editor.createWorld()
   else
      local w = initWorld()
      local lastTime = w.scene:getRealTime()
      local framerate = 30
      i=1
      while w.active do
	 
	 local now = w.scene:getRealTime()
	 local dt = now-lastTime
	 
	 framerate = 0.95 * framerate + 0.05/dt
	 
	 if i % 300 == 0 then
	    print("Framerate is " .. framerate)
	 end
	 
	 input.handle(w)
	 Screen.checkInput();
	 w:step(dt)
	 
	 boid.targetX = 100 - 60 * math.cos(w.scene.time*0.01)
	 boid.targetY = 40
	 boid.targetZ = w.scene:getHeight(boid.targetX, boid.targetY)+5
	 
	 boid:step(w.scene, dt)
	 w.scene:render()
	 Screen.swapBuffers()
	 
	 lastTime = now
	 i = i+1
      end
      Screen.destroy()
   end
end

