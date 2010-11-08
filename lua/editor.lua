module("editor", package.seeall)

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


function createWorld()
   local w = world.World.create("anna", false)
   w.scene:configure(2, 800, 500, 500)


   w.scene:setTerrainElement(
      10,1, 1, 
      1,
      0,1,0)
   terrainDiamond(
      w.scene, 10, 1, 1, (2^10) +1, (2^10) +1)
   
   
   print("Terrain created")
   
   w.scene:generateLod()
   print("LOD generated")
   
   w.scene:saveTerrain()
   print("Terrain saved")

--[[
   local bt = BallType.create(6)
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
   
   bt:save("anna", "ball1")
   
   local bt = BallType.create(4)
   for i = 1, (2^5) do
      for j = 1, (2^4) do
	 bt:setElement(i, j, 0.8 + 0.2*math.cos(j*math.pi*2 / (2^4)), 0.5, 0.4, 0.2)
      end
   end
   
   bt:calc()
   
   bt:save("anna", "torso1")
   bt:save("anna", "head1")
   bt:save("anna", "leftLeg1")
   bt:save("anna", "rightLeg1")
   bt:save("anna", "leftArm1")
   bt:save("anna", "rightArm1")

   print("Balls created and saved")
]]--

   print("Creating trees...")
   for i = 11, 790, 4 do
      for j = 11, 790, 4 do
	 local x = i+5*math.sin(0.1*j)
	 local y = j+5*math.sin(0.1*i)
	 
	 local h = w.scene:getTerrainElement(10,x,y) or 1000
	 if h < 10 and (x/30)%2 >= 1 then
	    w.scene:createTree(
	       "tree1", 
	       x, y,
	       (i*10+j*13)%360, 1 + 0.3*math.random());
	 end
      end
   end
   print("Trees created!")
   w.scene:saveItems()
   print("Trees saved")
end


