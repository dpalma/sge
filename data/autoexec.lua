-------------------------------------------------------------------------------
-- $Id$

bind("q", "quit();");
bind("escape", [[ShowModalDialog("maindlg.xml");]]);
bind("TAB", "EntityCenterOnRandom();");

gameMeshes = 
{
   "tree2.ms3d",
   "crate.ms3d",
   "face.3ds",
};

function GameInit()

   -- has to be done after the ground is set up
   ViewSetPos(0.5,0.5);

   nGameMeshes = table.getn(gameMeshes);

   for c=1,10 do
      local meshFileName = gameMeshes[math.random(nGameMeshes)];
      EntitySpawnTest(meshFileName, math.random(), math.random());
   end
end;

-------------------------------------------------------------------------------
