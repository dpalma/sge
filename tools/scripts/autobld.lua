-- $Id$

local f, osError;
f, osError = loadlib("autobld.dll", "init");
if f == nil then
   error("Error loading autobld.dll: " .. osError);
else
   f();
end

function pathcatpair(s,t)
   if string.len(s) == 0 then
      return t;
   end
   if string.len(t) == 0 then
      return s;
   end
   s0,sn,ss = string.find(s, ".*([/\\])$")
   t0,tn,ts = string.find(t, "^([/\\]).*");
   if not s0 and not t0 then
      return s .. "\\" .. t;
   elseif s0 and t0 then
      return s .. string.sub(t, 2);
   else
      return s .. t;
   end
end

function pathcat(...)
   local result = "";
   table.foreachi(arg, function(i,v) result = pathcatpair(result,v) end);
   return result;
end

nsis = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\NSIS");
--print("NSIS path is " .. nsis);

vc6 = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++", "ProductDir");
--print("VC++6 path is " .. vc6);

autobuildDir = "c:\\autobuild";

function genbuildlabel()
   return os.date("SGE%Y%m%d%H%M%S", os.time());
end

thisBuild = genbuildlabel();
thisBuildDir = pathcat(autobuildDir, thisBuild);
autobld:mkdir(thisBuildDir, true); -- fail if already exists

function emitcvsscript(script, cvscmd)
   if script then
      if not io.output(script) then
         error("An error occurred opening " .. script .. " for writing");
         exit(-1);
      end
   end

   local endl = "\n";

   io.write("@REM SCRIPT GENERATED ON " .. os.date() .. endl);
   io.write("@REM Build label: " .. thisBuild .. endl);
   io.write("@REM CVS command: " .. cvscmd .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("pushd " .. thisBuildDir .. endl);
   io.write("set CVSROOT=:pserver:anonymous@dpcvs.no-ip.org:/cvsstore" .. endl);
   io.write("cvs login <echo. 2>nul" .. endl);
   io.write(cvscmd .. endl);
   io.write("popd" .. endl);

   io.flush();

   if script then
      io.output(); -- close the file
   end
end

function runBuildScripts()
   local bldScriptsDir = pathcat(thisBuildDir, "sge\\tools\\scripts");
   for i,v in ipairs(autobld:dir(pathcat(bldScriptsDir, "bld.*.bat"))) do
      print(pathcat(bldScriptsDir, v) .. " " .. pathcat(thisBuildDir, "sge"));
      os.execute(pathcat(bldScriptsDir, v) .. " " .. pathcat(thisBuildDir, "sge"));
   end
end

function enumbuilddirs(f)
   for i,buildDir in ipairs(autobld:dir(pathcat(thisBuildDir, "sge", "Build.*"))) do
      s0,s1,s = string.find(buildDir, "Build%.(%w+)");
      if not s then
         error("Build directory " .. buildDir .. " not named as expected");
      end
      f(pathcat(thisBuildDir, "sge"), buildDir, s);
   end
end

function emitzipscript(srcdir, destdir, zip)
   assert(srcdir and destdir and zip);

   local batfile = pathcat(destdir, "makezip.bat");

   if not io.output(batfile) then
      error("An error occurred opening " .. batfile .. " for writing");
   end

   local endl = "\n";

   io.write("@REM SCRIPT GENERATED ON " .. os.date() .. endl);
   io.write("@REM Build label: " .. thisBuild .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("set PATH=" .. pathcat(thisBuildDir, "sge\\tools\\bin") .. ";%PATH%" .. endl);
   io.write("pushd " .. srcdir .. endl);

   local files = autobld:dir(pathcat(srcdir, "*.exe"));
   if not files then
      error("Empty file list: " ..  pathcat(srcdir, "*.exe"));
   end
   for i,f in ipairs(files) do
      io.write("minizip -9 -a " .. pathcat(destdir, zip) .. " " .. f .. endl);
   end

   local files = autobld:dir(pathcat(srcdir, "*.dll"));
   if not files then
      error("Empty file list: " ..  pathcat(srcdir, "*.dll"));
   end
   for i,f in ipairs(files) do
      io.write("minizip -a " .. pathcat(destdir, zip) .. " " .. f .. endl);
   end

   io.write("popd" .. endl);

   io.flush();
   io.output(); -- close the file
end

function emitzipscripts(run)
   enumbuilddirs(function(baseDir, buildDir, buildCompiler)
      local shipDir = pathcat(baseDir, "Ship." .. buildCompiler);
      autobld:mkdir(shipDir);
      for i,targetDir in ipairs(autobld:dir(pathcat(baseDir, buildDir, "*"))) do
         local shipTargetDir = pathcat(shipDir, targetDir);
         autobld:mkdir(shipTargetDir);
         local bin = pathcat(baseDir, buildDir, targetDir);
         emitzipscript(bin, shipTargetDir, string.lower(thisBuild .. targetDir .. ".zip"));
         if run then
            os.execute(pathcat(shipTargetDir, "makezip.bat"));
         end
      end
   end);
end

function buildinstallers()
   enumbuilddirs(function(baseDir, buildDir, buildCompiler)
      local shipDir = pathcat(baseDir, "Ship." .. buildCompiler);
      autobld:mkdir(shipDir);
      for i,targetDir in ipairs(autobld:dir(pathcat(baseDir, buildDir, "*"))) do
         local shipTargetDir = pathcat(shipDir, targetDir);
         autobld:mkdir(shipTargetDir);
         local buildBinDir = pathcat(baseDir, buildDir, targetDir);

         local instArgs = "/nocd ";
         instArgs = instArgs .. " /DBUILD_ROOT=" .. baseDir;
         instArgs = instArgs .. " /DBUILD_OUTDIR=" .. shipTargetDir;
         instArgs = instArgs .. " /DBUILD_BINARIES=" .. buildBinDir;
         instArgs = instArgs .. " /DBUILD_" .. string.upper(buildCompiler);
         instArgs = instArgs .. " /DBUILD_" .. string.upper(targetDir);

         local instCmd = string.format("%q", pathcat(nsis, "makensis.exe"));
         instCmd = instCmd .. " " .. "/O" .. pathcat(shipTargetDir, "makensis.log");
         instCmd = instCmd .. " " .. instArgs;
         instCmd = instCmd .. " " .. pathcat(baseDir, "installer\\sge.nsi");
         os.execute(instCmd);
      end
   end);
end

----------------------------------------

cvsgetscript = pathcat(thisBuildDir, "cvsget.bat");
cvstagscript = pathcat(thisBuildDir, "cvstag.bat");
emitcvsscript(cvsgetscript, "cvs co sge");
emitcvsscript(cvstagscript, "cvs tag -R " .. thisBuild);
os.execute(cvsgetscript);
runBuildScripts();
emitzipscripts(true);
buildinstallers();
