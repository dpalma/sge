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

bind("F7", [[GUIContext:ShowModalDialog("guitest.xml")]]);

--LogChannel([[GUILayout]]);
--LogChannel([[GUIButtonEvents]]);
--LogChannel([[GUIDialogEvents]]);
--LogChannel([[LuaInterp]]);
--LogChannel([[ResourceManager]]);

function LoadSampleLevel()
   GUIContext:Clear();
   GUIContext:Load("ingame.xml");
   
   SetTerrain("Rolling Grassy Hills.sgm");
   
   -- spawn this many entities
   nEntities = 15;

   models = ListResources("Model");
   table.foreachi(models, function(i, v) print("Model " .. i .. " is " .. v .. "\n") end);

   nModels = table.getn(models);
   for i = 1, nEntities do
      m = models[math.random(nModels)];
      EntitySpawnTest(m, math.random(), math.random());
   end
   
   -- Spawn an animated zombie and point the view at it
   EntitySpawnTest("zombie.ms3d", 0.5, 0.4);
   ViewSetPos(0.5, 0.4);
end;

function ShowSinglePlayerPage()
   LoadSampleLevel();
end;

function ShowOptionsPage()
end;

function ShowStartPage()
   GUIContext:Clear();
   GUIContext:Load("start.xml");
end;

-- Called automatically at start-up by the game engine
function GameInit()
   ShowStartPage()
end;

-------------------------------------------------------------------------------
