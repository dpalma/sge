-------------------------------------------------------------------------------
-- $Id$

bind("q", "quit();");
bind("escape", [[ConfirmedQuit("quitdlg.xml");]]);

bind("d", [[ToggleGUIDebugInfo(5, 200, "yellow");]]);

--LogChannel([[GUIButtonEvents]]);
--LogChannel([[GUIDialogEvents]]);

gameMeshes = 
{
   "tree2.ms3d",
   "crate.ms3d",
   "face.3ds",
   "zombie.ms3d",
};

function GameInit()
   LoadGUI("guitest.xml");

   SetTerrain("ground.tga", 0.2, "grass.tga");

   nGameMeshes = table.getn(gameMeshes);
   for i = 1, nGameMeshes do
	  for m=1,5 do
	     EntitySpawnTest(gameMeshes[i], math.random(), math.random());
	  end
   end
   
   EntitySpawnTest("zombie.ms3d", 0.5, 0.4);
   ViewSetPos(0.5, 0.4);
end;

-------------------------------------------------------------------------------
