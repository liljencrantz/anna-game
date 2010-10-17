module("anna", package.seeall)

require("lua/wrapper")
require("lua/scene")
require("lua/actor")
--require("strict")

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
   
--   BallPeer.create(sc.__peer, "ball1", 42, 42, 2, 0, 1);
   
   for i = 1, 100, 10 do
      for j = 1, 100, 10 do
	 TreePeer.create(sc.__peer, "tree1", i, j, (i+j)%360, 1);
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
