-------------------------------------------------------------------------------
-- $Id$

function QuitWithConfirm()
   if GUIContext:ShowModalDialog("confirmquit.xml") then
      quit();
   end
end;

bind("q", "quit();");
bind("escape", [[QuitWithConfirm();]]);

bind("F4", [[GUIContext:ToggleDebugInfo(5, 200, "yellow");]]);

bind("F7", [[GUIContext:ShowModalDialog("guitest.xml")]]);

--LogChannel([[GUILayout]]);
--LogChannel([[GUIButtonEvents]]);
--LogChannel([[GUIDialogEvents]]);
--LogChannel([[LuaInterp]]);
--LogChannel([[ResourceManager]]);

gameMeshes = 
{
   "tree2.ms3d",
   "crate.ms3d",
   "zombie.ms3d",
};

function LoadSampleLevel()
   GUIContext:Clear();
   GUIContext:Load("ingame.xml");

   SetTerrain("Rolling Grassy Hills.sgm");
   
   -- spawn this many entities
   nEntities = 15;

   nGameMeshes = table.getn(gameMeshes);
	for m=1,nEntities do
	   mesh = gameMeshes[math.random(nGameMeshes)];
	   EntitySpawnTest(mesh, math.random(), math.random());
	end
   
   -- Spawn an animated zombie and point the view at it
   EntitySpawnTest("zombie.ms3d", 0.5, 0.4);
   ViewSetPos(0.5, 0.4);
end;

-- Called automatically at start-up by the game engine
function GameInit()
   GUIContext:Clear();
   GUIContext:Load("start.xml");
end;

-------------------------------------------------------------------------------
