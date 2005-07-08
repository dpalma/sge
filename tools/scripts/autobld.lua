-- $Id$

local f, osError;
f, osError = loadlib("autobld.dll", "init");
if f == nil then
   error("Error loading autobld.dll: " .. osError);
else
   f();
end

local log = print;

function pathcatpair(s,t)
   if s == nil or string.len(s) == 0 then
      return t;
   end
   if t == nil or string.len(t) == 0 then
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

function genbuildlabel()
   return os.date("SGE%Y%m%d%H%M%S", os.time());
end

function emitcvsscript(script, cvscmd)
   if script then
      if not io.output(script) then
         error("An error occurred opening " .. script .. " for writing");
      end
   end

   local endl = "\n";

   io.write("@REM SCRIPT GENERATED ON " .. os.date() .. endl);
   io.write("@REM CVS command: " .. cvscmd .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("pushd " .. thisBuildDir .. endl);
   io.write("set CVSROOT=:sspi:anonymous@dpcvs.no-ip.org:/cvsstore" .. endl);
   io.write("cvs login <echo. 2>nul" .. endl);
   io.write(cvscmd .. endl);
   io.write("popd" .. endl);

   io.flush();

   if script then
      io.output(); -- close the file
   end
end

function buildvc6(bldroot)
   local vcdir = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++", "ProductDir");
   if not vcdir then
      error("Unable to determine Visual C++ 6.0 directory");
   end
   
   local script = pathcat(bldroot, "bldvc6.bat");
   if not io.output(script) then
      error("An error occurred opening " .. script .. " for writing");
   end

   local endl = "\n";

   io.write("@REM AUTO-GENERATED BUILD SCRIPT" .. endl);
   io.write("@REM " .. os.date() .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("call \"" .. pathcat(vcdir, "bin", "vcvars32.bat") .. "\"" .. endl);
   local dxsdkdir = os.getenv("DXSDK_DIR");
   if dxsdkdir then
      io.write("set INCLUDE=%INCLUDE%;\"" .. pathcat(dxsdkdir, "Include") .. "\"" .. endl);
      io.write("set LIB=%LIB%;\"" .. pathcat(dxsdkdir, "Lib") .. "\"" .. endl);
   end
   io.write("pushd " .. pathcat(bldroot, "sge\\vcproj\\vc6") .. endl);
   local cmdbase = "msdev 3rdparty.dsw /useenv /make \"AllTargets - ALL\"";
   io.write(cmdbase .. endl);
   cmdbase = "msdev sge.dsw /useenv /make \"AllTargets - Win32 Opt\" \"AllTargets - Win32 Debug\" \"AllTargets - Win32 Release\"";
   io.write(cmdbase .. endl);
   cmdbase = "msdev sgestatic.dsw /useenv /make \"AllTargets - Static\"";
   io.write(cmdbase .. endl);
   io.write("popd" .. endl);

   io.flush();
   io.output(); -- close the file

   os.execute(script);
end

function buildvc71(bldroot)
   local vcdir = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VisualStudio\\7.1\\Setup\\VC", "ProductDir");
   if not vcdir then
      error("Unable to determine Visual C++ .NET 2003 directory");
   end

   local script = pathcat(bldroot, "bldvc71.bat");
   if not io.output(script) then
      error("An error occurred opening " .. script .. " for writing");
   end

   local endl = "\n";

   io.write("@REM AUTO-GENERATED BUILD SCRIPT" .. endl);
   io.write("@REM " .. os.date() .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("call \"" .. pathcat(vcdir, "bin", "vcvars32.bat") .. "\"" .. endl);
   local dxsdkdir = os.getenv("DXSDK_DIR");
   if dxsdkdir then
      io.write("set INCLUDE=%INCLUDE%;\"" .. pathcat(dxsdkdir, "Include") .. "\"" .. endl);
      io.write("set LIB=%LIB%;\"" .. pathcat(dxsdkdir, "Lib") .. "\"" .. endl);
   end
   io.write("pushd " .. pathcat(bldroot, "sge\\vcproj\\vc71") .. endl);
   local cmdbase = "devenv 3rdparty.sln /useenv";
   io.write(cmdbase .. " /build Opt" .. endl);
   io.write(cmdbase .. " /build Debug" .. endl);
   io.write(cmdbase .. " /build Release" .. endl);
   io.write(cmdbase .. " /build StaticDebug" .. endl);
   io.write(cmdbase .. " /build StaticRelease" .. endl);
   cmdbase = "devenv sge.sln /useenv";
   io.write(cmdbase .. " /build Opt" .. endl);
   io.write(cmdbase .. " /build Debug" .. endl);
   io.write(cmdbase .. " /build Release" .. endl);
   cmdbase = "devenv sgestatic.sln /useenv";
   io.write(cmdbase .. " /build StaticDebug" .. endl);
   io.write(cmdbase .. " /build StaticRelease" .. endl);
   io.write("popd" .. endl);

   io.flush();
   io.output(); -- close the file

   os.execute(script);
end

function buildvc2005express(bldroot)
   local vcdir = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VCExpress\\8.0\\Setup\\VC", "ProductDir");
   if not vcdir then
      error("Unable to determine Visual C++ 2005 Express directory");
   end

   local script = pathcat(bldroot, "bldvc2005express.bat");
   if not io.output(script) then
      error("An error occurred opening " .. script .. " for writing");
   end

   local endl = "\n";

   io.write("@REM AUTO-GENERATED BUILD SCRIPT" .. endl);
   io.write("@REM " .. os.date() .. endl);
   io.write("@echo off" .. endl);
   io.write("setlocal" .. endl);
   io.write("call \"" .. pathcat(vcdir, "bin", "vcvars32.bat") .. "\"" .. endl);
   local w32sdkDir = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Win32SDK\\Directories", "Install Dir");
   if w32sdkDir then
      io.write("set INCLUDE=%INCLUDE%;\"" .. pathcat(w32sdkDir, "include") .. "\"" .. endl);
      io.write("set LIB=%LIB%;\"" .. pathcat(w32sdkDir, "lib") .. "\"" .. endl);
   end
   local dxsdkdir = os.getenv("DXSDK_DIR");
   if dxsdkdir then
      io.write("set INCLUDE=%INCLUDE%;\"" .. pathcat(dxsdkdir, "Include") .. "\"" .. endl);
      io.write("set LIB=%LIB%;\"" .. pathcat(dxsdkdir, "Lib") .. "\"" .. endl);
   end
   io.write("pushd " .. pathcat(bldroot, "sge\\vcproj\\vc2005express") .. endl);
   local cmdbase = "vcexpress 3rdparty.sln /useenv";
   io.write(cmdbase .. " /build \"Opt|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"Debug|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"Release|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"StaticDebug|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"StaticRelease|Win32\"" .. endl);
   cmdbase = "vcexpress sge.sln /useenv";
   io.write(cmdbase .. " /build \"Opt|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"Debug|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"Release|Win32\"" .. endl);
   cmdbase = "vcexpress sgestatic.sln /useenv";
   io.write(cmdbase .. " /build \"StaticDebug|Win32\"" .. endl);
   io.write(cmdbase .. " /build \"StaticRelease|Win32\"" .. endl);
   io.write("popd" .. endl);

   io.flush();
   io.output(); -- close the file

   os.execute(script);
end

function enumbuilddirs(bldroot, f)
   for i,buildDir in ipairs(autobld:dir(pathcat(bldroot, "sge", "Build.*"))) do
      s0,s1,s = string.find(buildDir, "Build%.(%w+)");
      if not s then
         error("Build directory " .. buildDir .. " not named as expected");
      end
      f(pathcat(bldroot, "sge"), buildDir, s);
   end
end

function buildzips(bldroot)
   local wzzip = string.format("%q", autobld:getregistryvalue(
      HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wzzip.exe"));
   if wzzip then
      enumbuilddirs(bldroot, function(baseDir, buildDir, buildCompiler)
         local shipDir = pathcat(baseDir, "Ship." .. buildCompiler);
         autobld:mkdir(shipDir);
         for i,targetDir in ipairs(autobld:dir(pathcat(baseDir, buildDir, "*"))) do
            local shipTargetDir = pathcat(shipDir, targetDir);
            autobld:mkdir(shipTargetDir);
            local bin = pathcat(baseDir, buildDir, targetDir);
            local params = pathcat(shipTargetDir, string.lower(thisBuild .. targetDir .. ".zip"));
            params = params .. " -ybc"; -- say yes to any prompts
            params = params .. " " .. pathcat(bin, "*.exe");
            params = params .. " " .. pathcat(bin, "*.dll");
            autobld:spawn(wzzip .. " " .. params);
         end
      end);
   end
end

function buildinstallers(bldroot)
   local nsis = autobld:getregistryvalue(HKEY_LOCAL_MACHINE, "Software\\NSIS");
   enumbuilddirs(bldroot, function(baseDir, buildDir, buildCompiler)
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
         autobld:spawn(instCmd);
      end
   end);
end

local buildfns =
{
   vc6 = buildvc6,
   vc71 = buildvc71,
   vc2005express = buildvc2005express,
}

function realbuild(autobuildDir)
   thisBuild = genbuildlabel();
   thisBuildDir = pathcat(autobuildDir, thisBuild);
   autobld:mkdir(thisBuildDir, true); -- fail if already exists

   cvsgetscript = pathcat(thisBuildDir, "cvsget.bat");
   cvstagscript = pathcat(thisBuildDir, "cvstag.bat");
   emitcvsscript(cvsgetscript, "cvs co sge");
   emitcvsscript(cvstagscript, "cvs tag -R " .. thisBuild);
   os.execute(cvsgetscript);

   local spec = pathcat(thisBuildDir, "sge\\vcproj\\*");
   for i,v in ipairs(autobld:dir(spec)) do
      local f = buildfns[v];
      if f then
         if not pcall(f, thisBuildDir) then
            log("Building " .. v .. " failed");
         end;
      end
   end

   buildinstallers(thisBuildDir);
   buildzips(thisBuildDir);
end

realbuild("c:\\autobld");
