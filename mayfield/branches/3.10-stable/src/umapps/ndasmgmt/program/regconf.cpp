<?xml version="1.0" encoding="Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.10"
	Name="ndasmgmt"
	ProjectGUID="{F42DAA8A-2A43-4F8A-818D-CFDB1AEB7C48}"
	RootNamespace=""
	Keyword="MakeFileProj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="objchk_w2k_x86\i386"
			IntermediateDirectory="."
			ConfigurationType="0">
			<Tool
				Name="VCNMakeTool"
				BuildCommandLine="ddkbuild -WNET2K chk . -e"
				ReBuildCommandLine="ddkbuild -WNET2K chk . -ceZ"
				CleanCommandLine="ddkbuild -WNET2K chk . -c0"
				Output="objchk_w2k_x86\i386\ndasmgmt.exe"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="objfre_w2k_x86\i386"
			IntermediateDirectory="."
			ConfigurationType="0">
			<Tool
				Name="VCNMakeTool"
				BuildCommandLine="ddkbuild -WNET2K fre . -e"
				ReBuildCommandLine="ddkbuild -WNET2K fre . -ceZ"
				CleanCommandLine="ddkbuild -WNET2K fre . -c0"
				Output="objfre_w2k_x86\i386\ndasmgmt.exe"/>
		</Configuration>
	</Configurations>
	<References>
	</References>
	<Files>
		<Filter
			Name="Source Files"
			Filter="cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
			UniqueIdentifier="{4FC737F1-C7A5-4376-A066-2A32D752A2FF}">
			<File
				RelativePath=".\aboutdlg.cpp">
			</File>
			<File
				RelativePath=".\adminprivs.cpp">
			</File>
			<File
				RelativePath=".\appconf.cpp">
			</File>
			<File
				RelativePath=".\apperrdlg.cpp">
			</File>
			<File
				RelativePath=".\argv.cpp">
			</File>
			<File
				RelativePath=".\confirmdlg.cpp">
			</File>
			<File
				RelativePath=".\devpropsh.cpp">
			</File>
			<File
				RelativePath=".\devregdlg.cpp">
			</File>
			<File
				RelativePath=".\devregwiz.cpp">
			</File>
			<File
				RelativePath=".\eventhandler.cpp">
			</File>
			<File
				RelativePath=".\mainframe.cpp">
			</File>
			<File
				RelativePath=".\menubitmap.cpp">
			</File>
			<File
				RelativePath=".\muisel.cpp">
			</File>
			<File
				RelativePath=".\ndascls.cpp">
			</File>
			<File
				RelativePath=".\ndasmgmt.cpp">
			</File>
			<File
				RelativePath=".\ndastypestr.cpp">
			</File>
			<File
				RelativePath=".\nmmenu.cpp">
			</File>
			<File
				RelativePath=".\optionpsh.cpp">
			</File>
			<File
				RelativePath=".\regconf.cpp">
			</File>
			<File
				RelativePath=".\singleinst.cpp">
			</File>
			<File
				RelativePath=".\stdafx.cpp">
			</File>
			<File
				RelativePath=".\syshelp.cpp">
			</File>
		</Filter>
		<Filter
			Name="Header Files"
			Filter="h;hpp;hxx;hm;inl;inc;xsd"
			UniqueIdentifier="{93995380-89BD-4b04-88EB-625FBE52EBFB}">
			<File
				RelativePath=".\aboutdlg.h">
			</File>
			<File
				RelativePath=".\adminprivs.h">
			</File>
			<File
				RelativePath=".\appconf.h">
			</File>
			<File
				RelativePath=".\apperrdlg.h">
			</File>
			<File
				RelativePath=".\appiniterr.h">
			</File>
			<File
				RelativePath=".\argv.h">
			</File>
			<File
				RelativePath=".\confirmdlg.h">
			</File>
			<File
				RelativePath=".\devpropsh.h">
			</File>
			<File
				RelativePath=".\devregdlg.h">
			</File>
			<File
				RelativePath=".\devregwiz.h">
			</File>
			<File
				RelativePath=".\eventhandler.h">
			</File>
			<File
				RelativePath=".\mainframe.h">
			</File>
			<File
				RelativePath=".\menubitmap.h">
			</File>
			<File
				RelativePath=".\muisel.h">
			</File>
			<File
				RelativePath=".\mytaskbaricon.h">
			</File>
			<File
				RelativePath=".\ndascls.h">
			</File>
			<File
				RelativePath=".\ndasdevice.h">
			</File>
			<File
				RelativePath=".\ndasmgmt.h">
			</File>
			<File
				RelativePath=".\ndastypestr.h">
			</File>
			<File
				RelativePath=".\nmmenu.h">
			</File>
			<File
				RelativePath=".\observer.h">
			</File>
			<File
				RelativePath=".\optionpsh.h">
			</File>
			<File
				RelativePath=".\pix.h">
			</File>
			<File
				RelativePath=".\regconf.h">
			</File>
			<File
				RelativePath=".\singleinst.h">
			</File>
			<File
				RelativePath=".\stdafx.h">
			</File>
			<File
				RelativePath=".\syshelp.h">
			</File>
			<File
				RelativePath=".\taskbariconex.h">
			</File>
			<File
				RelativePath=".\waitdlg.h">
			</File>
			<File
				RelativePath=".\worker.h">
			</File>
		</Filter>
		<Filter
			Name="Resource Files"
			Filter="rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
			UniqueIdentifier="{67DA6AB6-F800-4c08-8B7A-83BB121AAD01}">
			<File
				RelativePath=".\ndasmgmt_app.rc">
			</File>
			<File
				RelativePath=".\ndasmgmt_appiniterr.h">
			</File>
			<File
				RelativePath=".\ndasmgmt_appiniterr.rc2">
			</File>
		</Filter>
		<File
			RelativePath=".\cmdline.txt">
		</File>
		<File
			RelativePath=".\makefile">
		</File>
		<File
			RelativePath=".\makefile.inc">
		</File>
		<File
			RelativePath=".\sources">
		</File>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>