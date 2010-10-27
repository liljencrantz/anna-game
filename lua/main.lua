module("anna", package.seeall)

require("lua/util")
require("lua/wrapper")
require("lua/scene")
require("lua/actor")
require("math")

--[[
    if(screen_key_get('q'))
    {
	screen_exit=1;	
	exit(1);
    }

    if(screen_key_get('0'))
    {
	w->render_quality += 0.1;
    }
    
    if(screen_key_get('9'))
    {
	w->render_quality -= 0.1;
    }
    
    if(screen_key_get('8'))
    {
	w->camera.lr_rot += 2;
	if(w->camera.lr_rot>= 360.0)
	{
	    w->camera.lr_rot -= 360;
	}
    }
    
    if(screen_key_get('7'))
    {
	w->camera.lr_rot -= 2;
	if(w->camera.lr_rot< 0.0)
	{
	    w->camera.lr_rot += 360;
	}
	
    }
    
    actor_set_action(w->player, ACTION_WALK_FORWARD, screen_key_get(SDLK_UP));	
    actor_set_action(w->player, ACTION_WALK_BACKWARD, screen_key_get(SDLK_DOWN));	
    actor_set_action(w->player, ACTION_TURN_LEFT, screen_key_get(SDLK_LEFT));	
    actor_set_action(w->player, ACTION_TURN_RIGHT, screen_key_get(SDLK_RIGHT));	

}
]]--

require("lua/input")

function run()
   local sc = scene.Scene.create(8, 200)
   
   sc.player = actor.Actor.create(sc, "Bosse");
   sc.camera = {100, 100, 10}

   BoidSetPeer.create(sc.__peer, 40, 40, 40);
   bid = BallPeer.create(sc.__peer, "ball1", 2)
   bid:setLocation(
      sc.__peer, 
      42,42,3,
      40,0,0)
   for i = 11, 200, 4 do
      for j = 11, 200, 4 do
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

      framerate = 0.98 * framerate + 0.02/dt
      
      if i % 300 == 0 then
	 print("Framerate is " .. framerate)
      end
      input.handle(sc)
      Screen.checkInput();
      sc:step(dt)
      sc:render()
      Screen.swapBuffers()
      
      lastTime = now
      i = i+1
   end
   Screen.destroy()
end


