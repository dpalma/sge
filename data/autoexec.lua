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
   SetTerrain(map, "ingame.xml");
end;

frameStatsOverlay = [[<page><label renderer="basic" id="frameStats" style="width:50%;height:25%;foreground-color:white" /></page>]];

-- Called automatically at start-up by the game engine
function GameInit()
   GUIEventSounds:SetEventSound("click", "click.wav");
   GUIContext:PushPage("start.xml");
   GUIContext:AddOverlay(frameStatsOverlay);
end;

-------------------------------------------------------------------------------
