module("editor", package.seeall)

function florp()
   return 0.09*(2*math.random()-1.0)
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


function createWorld()
   local w = world.World.create("anna", false)
   w.scene:configure(2, 800, 500, 500)
   w.scene:setTerrainElement(
      10,1, 1, 
      1,
      0,1,0)
   terrainDiamond(
      w.scene, 10, 1, 1, (2^10) +1, (2^10) +1)
   
   for i = 1, 2^10 do
      for j = 1, 2^10 do
	 
	 local h = w.scene:getTerrainElement(10,i,j) or 1000
	 dx, dy = w.scene:getSlope(i,j)
	 dd = math.sqrt(dx*dx + dy*dy)
	 c = {0.2,0.4,0.1}
	 if dd > 1 then
	    c = {0.3,0.3,0.3}
	 end
	 
	 w.scene:setTerrainElement(10,i,j,h, unpack(c))
	 
      end
   end
   print("Terrain created")
   
   w.scene:generateLod()
   print("LOD generated")
   
   w.scene:saveTerrain()
   print("Terrain saved")


   local bt = BallType.create(6, "ball1",0.65)
   for i = 1, (2^7) do
      for j = 1, (2^6) do
	 
	 local bumpX = math.sin(3*i/(2^7)*math.pi*4)
	 local bumpY = math.sin(2*j/(2^6)*math.pi*4)
	 local bump =  math.abs(bumpX + bumpY)
	 
	 local scale = 
	    function(val, bump)
	       return (1 + 0.2*bump)*val
	    end
	 local c = {
	    scale(0.1,bump),
	    scale(0.3,bump),
	    scale(0.05,bump)}
	 
	 bt:setElement(i, j, 0.8 + 0.1*bump, unpack(c))
      end
   end
   
   bt:calc()
   bt:save(w.scene)
   
   local bt = BallType.create(4, "torso1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   local bt = BallType.create(4, "rightArm1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   local bt = BallType.create(4, "leftArm1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   local bt = BallType.create(4, "rightLeg1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   local bt = BallType.create(4, "leftLeg1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   local bt = BallType.create(4, "head1",1)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(
	    i, j, 
	    0.9 + 0.1*math.cos(j*math.pi*2 / (2^4)), 
	    0.5, 0.4, 0.2)
      end
   end
   bt:calc()   
   bt:save(w.scene)

   print("Balls created and saved")

   print("Creating trees...")
   for i = 11, 790, 12 do
      for j = 11, 790, 12 do
	 local x = i+5*math.sin(0.1*j)
	 local y = j+5*math.sin(0.1*i)
	 
	 local h = w.scene:getTerrainElement(10,x,y) or 1000
	 if h < 10 and (i/40)%2 >= 1 then
	    w:createTree(
	       "tree1", 
	       {x,y,h},
	       1 + 0.3*math.random())
	 end
      end
   end
   print("Trees created!")
   w:saveItems()
   print("Trees saved")
end


