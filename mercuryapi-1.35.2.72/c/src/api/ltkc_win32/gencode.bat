rem #
rem # This runs xsltproc.exe to generate two files used by LTKCPP.
rem #
rem # You can download xsltproc.EXE from this URL
rem # http://www.zlatkovic.com/libxml.en.html
rem #
rem #

set PATH=%PATH%;"xsltproc"


set LLRPDEF=Definitions\Core\llrp-1x0-def.xml

xsltproc --stringparam RegistryName Core -o inc\out_ltkc.h inc\ltkc_gen_h.xslt %LLRPDEF%
xsltproc --stringparam RegistryName Core -o inc\out_ltkc.inc inc\ltkc_gen_c.xslt %LLRPDEF% 

set LLRPORGDEF=Definitions\LLRP.org\llrp-control-def.xml
if not exist %LLRPORGDEF% (
  copy ..\..\..\..\..\tmmpd\common\llrp-control-def.xml %LLRPORGDEF%
)

xsltproc --stringparam RegistryName Tm -o inc\out_tm_ltkc.h inc\ltkc_gen_h.xslt %LLRPORGDEF%
xsltproc --stringparam RegistryName Tm -o inc\out_tm_ltkc.inc inc\ltkc_gen_c.xslt %LLRPORGDEF% 
