-------------------------------------------------------------------------------
-- $Id$

grass =
{
    name = "Grass",
    texture = "grass.tga",
    horizontalImages = 4,
    verticalImages = 4,
};

function EditorInit()
   RegisterTileTexture(grass);
end;
