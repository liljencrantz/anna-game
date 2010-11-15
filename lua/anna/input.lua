module("input", package.seeall)

ARROW_LEFT = 276
ARROW_DOWN = 274
ARROW_RIGHT = 275
ARROW_UP = 273


function handle(world)
   f=anna.Screen.keyGet
   a=actor.actions
   p = world.player
   
   if f('q') then
      world.active=false
   end
   if f(48) then
      world.scene.renderQuality = world.scene.renderQuality *1.003
      print(world.scene.renderQuality)
   end

   if f(57) then
      world.scene.renderQuality = world.scene.renderQuality /1.003
      print(world.scene.renderQuality)
   end

   p.actions.walk_forward = f(ARROW_UP)
   p.actions.walk_backward = f(ARROW_DOWN)

   p.actions.turn_left = f(ARROW_LEFT)
   p.actions.turn_right = f(ARROW_RIGHT)
end

