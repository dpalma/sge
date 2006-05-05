--------------------------------------------------------------------------------
-- $Id$
--
-- GUI script code for customgame.xml

maps = ListResources("SGE.Map", 1); -- the one tells the resource manager not to load each
if maps and (table.getn(maps) > 0) then
	local mapsList = GUIContext:GetElement("mapsList");
	if mapsList then
		table.foreachi(maps, function(i, v) mapsList:AddItem(v) end);
	end;
end;

function getSelectedMap()
	local mapsList = GUIContext:GetElement("mapsList");
	if mapsList then
		local sel = mapsList:GetSelected();
		if sel then
			return mapsList:GetItem(sel);
		end;
	end;
	return nil;
end;

function OnStart()
	local map = getSelectedMap();
	if map then
		LoadSampleLevel(map);
	end;
end;

function FillMapPropertyControls(title, author, descrip, numPlayers)
	local titleCtrl = GUIContext:GetElement("title");
	if titleCtrl then
		titleCtrl:SetText(title);
	end;
	local authorCtrl = GUIContext:GetElement("author");
	if authorCtrl then
		authorCtrl:SetText(author);
	end;
	local descrCtrl = GUIContext:GetElement("descr");
	if descrCtrl then
		descrCtrl:SetText(descrip);
	end;
	local numPlayersCtrl = GUIContext:GetElement("numPlayers");
	if numPlayersCtrl then
		if numPlayers then
			numPlayersCtrl:SetText(tostring(numPlayers));
		else
			numPlayersCtrl:SetText(nil);
		end;
	end;
end;

function OnMapsListSelChange()
	local map = getSelectedMap();
	if map then
		local title, author, descrip, numPlayers = GetMapProperties(map);
		FillMapPropertyControls(title, author, descrip, numPlayers);
	end;
end;
