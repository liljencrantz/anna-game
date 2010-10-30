module("anna", package.seeall)

require("lua/util")
require("lua/wrapper")
require("lua/scene")
require("lua/actor")
require("lua/input")
require("math")

function run()
   local sc = scene.Scene.create(8, 200)
   
   sc.player = actor.Actor.create(sc, "Bosse");
   sc.camera = {100, 100, 10}
   
   boid = BoidSetPeer.create(sc.__peer, 40, 40, 40);
   
   for i = 11, 200, 3 do
      for j = 11, 200, 15 do
	 if true then
	    local x = i+5*math.sin(0.1*j)
	    local y = j+5*math.sin(0.1*i)
	    bid = BallPeer.create(sc.__peer, "ball1", 1.5)
	    bid:setLocation(
	       sc.__peer, 
	       x,y,
	       sc:getHeight(x,y)+3.5,
	       40,0,0)
--	    else
	    TreePeer.create(sc.__peer, "tree1", i+5*math.sin(0.1*j), j+5*math.sin(0.1*i), (i*10+j*13)%360, 1);
	    end
      end
   end

   local lastTime = sc:getRealTime()
   local framerate = 30
   i=1
   while sc.active do

      local now = sc:getRealTime()
      local dt = now-lastTime

      framerate = 0.95 * framerate + 0.05/dt
      
      if i % 300 == 0 then
	 print("Framerate is " .. framerate)
      end
      input.handle(sc)
      Screen.checkInput();
      sc:step(dt)
      
      boid.targetX = 100 - 60 * math.cos(sc.time*0.01)
      boid.targetY = 40
      boid.targetZ = sc:getHeight(boid.targetX, boid.targetY)+5

      boid:step(sc.__peer, dt)
      sc:render()
      Screen.swapBuffers()
      
      lastTime = now
      i = i+1
   end
   Screen.destroy()
end

