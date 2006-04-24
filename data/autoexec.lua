-------------------------------------------------------------------------------
-- $Id$

function QuitWithConfirm()
   if GUIContext:ShowModalDialog("confirmquit.xml") then
      quit();
   end
end;

bind("q", "quit();");
bind("escape", [[QuitWithConfirm();]]);

bind("F4", [[GUIContext:ToggleDebugInfo(5, 100, "font-name:Courier New;font-pointsize:8;foreground-color:yellow;");]]);

--LogChannel([[GUILayout]]);
--LogChannel([[GUIButtonEvents]]);
--LogChannel([[GUIDialogEvents]]);
--LogChannel([[LuaInterp]]);
--LogChannel([[ResourceManager]]);

function LoadSampleLevel(map)
   GUIContext:PushPage("ingame.xml");
   SetTerrain(map);
end;

function ShowCustomGamePage()
   GUIContext:PushPage("customgame.xml");
end;

-- Called automatically at start-up by the game engine
function GameInit()
   GUIEventSounds:SetEventSound("click", "click.wav");
   GUIContext:PushPage("start.xml");
end;

-------------------------------------------------------------------------------
