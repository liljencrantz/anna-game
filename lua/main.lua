module("anna", package.seeall)

require("lua/util")
require("lua/world")
require("lua/actor")
require("lua/input")
require("math")
require("lua/editor")

function initWorld()

   local w = world.World.create("anna", true)
   w.player = actor.Actor.create(w, "Bosse");
   
   actor.Actor.create(w, "Hasse");
   
   boid = BoidSetPeer.create(w.scene, 40, 40, 40);

   local b = BallPeer.create(w.scene, "ball1", 5);
   b:setLocation(w.scene, 40,40,6,0,0,0);
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

