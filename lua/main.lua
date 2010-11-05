module("anna", package.seeall)

require("lua/util")
require("lua/world")
require("lua/actor")
require("lua/input")
require("math")

function florp()
    return 0.09*(2*math.random()-1.0);
end


function terrainDiamond(sc, lvl, x1, y1, x2, y2)
   if ((x2-x1) < 2) or ((y2-y1) < 2)then
      return
   end
   local xm = (x1+x2)/2
   local ym = (y1+y2)/2

   local d = x2-x1
   local hx1y1 = sc:getTerrainElement(lvl,x1,y1) or 0
   local hx1y2 = sc:getTerrainElement(lvl,x1,y2) or 0
   local hx2y1 = sc:getTerrainElement(lvl,x2,y1) or 0
   local hx2y2 = sc:getTerrainElement(lvl,x2,y2) or 0
--   print(string.format("%.2f %.2f %.2f %.2f", hx1y1, hx2y1, hx1y2, hx2y2))

   sc:setTerrainElement(
      lvl,xm,y1, 
      (hx1y1+hx2y1)*0.5 + florp()*d, 
      0.2, 0.5, 0.1)
   sc:setTerrainElement(
      lvl,x1,ym, 
      (hx1y1+hx1y2)*0.5 + florp()*d, 
      0.2, 0.5, 0.1)
   sc:setTerrainElement(
      lvl,xm,ym, 
      (hx1y1+hx2y1+hx1y2+hx2y2)*0.25 + florp()*d, 
      0.2, 0.5, 0.1)
   terrainDiamond(sc, lvl, xm, ym, x2, y2)
   terrainDiamond(sc, lvl, x1, ym, xm, y2)
   terrainDiamond(sc, lvl, xm, y1, x2, ym)
   terrainDiamond(sc, lvl, x1, y1, xm, ym)
end

function run()
   local w = nil

   if false then
      w = world.World.create("anna", false)
      w.scene:configure(2, 800)
      
      w.scene:setTerrainElement(
	 10,1, 1, 
	 1,
	 0,1,0)
      terrainDiamond(
	 w.scene, 10, 1, 1, (2^10) +1, (2^10) +1)
      
--   for i=1,2^8 do
--      for j=1,2^8 do
--	 sc:setTerrainElement(8,i, j, math.sin(i*0.1)+math.sin(j*0.1), 0,1,0)
--      end
--   end
      print("Terrain created")
      
      w.scene:generateLod()
      print("LOD generated")
      
      w.scene:save()
   else
      w = world.World.create("anna", true)
   end
   w.player = actor.Actor.create(w, "Bosse");
   
   
   boid = BoidSetPeer.create(w.scene, 40, 40, 40);
   
   for i = 11, 200, 3 do
      for j = 11, 200, 15 do
	 if true then
	    local x = i+5*math.sin(0.1*j)
	    local y = j+5*math.sin(0.1*i)
	    bid = BallPeer.create(w.scene, "ball1", 1.5)
	    bid:setLocation(
	       w.scene, 
	       x,y,
	       w.scene:getHeight(x,y),
	       40,0,0)
--	    else
	    TreePeer.create(w.scene, "tree1", i+5*math.sin(0.1*j), j+5*math.sin(0.1*i), (i*10+j*13)%360, 1);
	 end
      end
   end

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

