module("input", package.seeall)

require("anna")
require("actor")

ARROW_LEFT = 276
ARROW_DOWN = 274
ARROW_RIGHT = 275
ARROW_UP = 273


function handle(scene)
   
   f=anna.Screen.keyGet
   a=actor.actions
   p = scene.player
   
   if f('q') then
      scene.active=false
   end

   p.actions.walk_forward = f(ARROW_UP)
   p.actions.walk_backward = f(ARROW_DOWN)
   p.actions.turn_left = f(ARROW_LEFT)
   p.actions.turn_right = f(ARROW_RIGHT)
   
end

