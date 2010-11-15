
guiObj = gui.Gui()

grp = guiObj.addButtonGroup({1,0}, gui.EXPAND_UP)


for _,action in ipairs(player.allActions) do

  grp.add{

     label=action.name,

     image={sprintf("buttons/actions/%s.png", action.slug), sprintf("buttons/actions/%s.png", action.category)},

     color={0,2,0,5,0,1},

     callback=function(event)

        scene.player:addAction(action)

     end

  }

end
