-------------------------------------------------------------------------------
-- $Id$

bind("q", "quit();");
bind("escape", [[ShowModalDialog("maindlg.xml");]]);

gameMeshes = 
{
   "tree.ms3d",
   "crate.ms3d",
   "face.3ds",
   "zombie.ms3d",
};

function GameInit()
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
