$ErrorActionPreference = "Stop"

$imgPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath("build/boot.vhd")
$flagRemovePrevious = $false

# Scan for each arguments.
for ($i = 0; $i -lt $args.Count; $i++) {
	switch ($args[$i]) {
		"--remove-previous" {
			$flagRemovePrevious = $true
		}
		Default {
			Write-Warning "Invalid argument ``$($args[$i])'"
		}
	}
}

function ExecDiskpartScript([string]$content) {
	if (-not $dpScriptFile) {
		$dpScriptFile = New-TemporaryFile
	}
	Clear-Content $dpScriptFile
	Write-Output $content | Out-File -Encoding "ascii" $dpScriptFile.FullName
	diskpart /s $dpScriptFile.FullName
}

function CreateDiskImage {
	ExecDiskpartScript "`
create vdisk MAXIMUM=96 FILE=`"$($imgPath)`"`n`
select vdisk FILE=`"$($imgPath)`"`n`
attach vdisk`n`
create partition PRIMARY SIZE=64 OFFSET=1024`n`
select partition 1`n`
format FS=FAT32 QUICK`n`
assign letter=B`n`
"
}

function InstallGrub {
	New-Item "B:\sys\" -ItemType dir
	New-Item "B:\sys\boot\" -ItemType dir
	New-Item "B:\sys\kernel\" -ItemType dir

	grub-install `
		--target=i386-pc `
		--skip-fs-probe `
		--no-rs-codes `
		--boot-directory="B:\sys\boot" `
		"\\.\PhysicalDrive$($(Get-Partition -DriveLetter B).DiskNumber)"
}

function UpdateGrubConfig {
	Copy-Item test\bootimg\config\grub.cfg B:\sys\boot\grub\
	Get-Content -Raw test\bootimg\config\49_pbos | Out-File -Encoding "utf8" -Append B:\sys\boot\grub\grub.cfg
}

function CopySystemFiles {
	Copy-Item build\pbkim B:\sys\kernel\
	Copy-Item build\initcar B:\sys\boot\
}

function RemovePreviousDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
detach vdisk`n"

	Remove-Item $imgPath
}

function MountDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
attach vdisk`n`
select partition 1`n`
assign letter=B`n`
"
}

function UnmountDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
detach vdisk`n"
}

if ($flagRemovePrevious) {
	RemovePreviousDiskImage
}

if (Test-Path "B:") {
	Write-Output "Error: Drive letter B is already in use"
	Exit -1;
}

if (-not (Test-Path $imgPath)) {
	CreateDiskImage
	InstallGrub
} else {
	MountDiskImage
}

UpdateGrubConfig
CopySystemFiles

UnmountDiskImage
