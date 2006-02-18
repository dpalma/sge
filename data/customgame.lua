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

function OnMapsListSelChange()
	local map = getSelectedMap();
	if map then
		local title, author, descrip, numPlayers = GetMapProperties(map);
		if title then
			local lbl = GUIContext:GetElement("title");
			if lbl then
				lbl:SetText(title);
			end;
		end;
		if author then
			local lbl = GUIContext:GetElement("author");
			if lbl then
				lbl:SetText(author);
			end;
		end;
		if descrip then
			local lbl = GUIContext:GetElement("descr");
			if lbl then
				lbl:SetText(descrip);
			end;
		end;
		if numPlayers then
			local lbl = GUIContext:GetElement("numPlayers");
			if lbl then
				lbl:SetText(tostring(numPlayers));
			end;
		end;
	end;
end;
