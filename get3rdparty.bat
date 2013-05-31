@echo off

if not exist 3rdparty (
   mkdir 3rdparty
)

pushd 3rdparty

if not exist tinyxml (
   curl --location -O http://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip
   unzip tinyxml_2_6_2.zip
   del tinyxml_2_6_2.zip
)

if not exist UnitTest++ (
   curl --location -O  http://downloads.sourceforge.net/project/unittest-cpp/UnitTest++/1.4/unittest-cpp-1.4.zip
   unzip unittest-cpp-1.4.zip
   del unittest-cpp-1.4.zip
)

popd
